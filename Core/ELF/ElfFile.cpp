#include "stdafx.h"
#include "ElfFile.h"
#include <vector>
#include <algorithm>
#include "Core/Misc.h"

#ifndef _WIN32
#define _stricmp strcasecmp
#endif

static bool stringEqualInsensitive(const std::string& a, const std::string& b)
{
	if (a.size() != b.size())
		return false;
	return _stricmp(a.c_str(),b.c_str()) == 0;
}

bool compareSection(ElfSection* a, ElfSection* b)
{
	return a->getOffset() < b->getOffset();
}

ElfSection::ElfSection(Elf32_Shdr header): header(header)
{
	owner = NULL;
}

void ElfSection::setOwner(ElfSegment* segment)
{
	header.sh_offset -= segment->getOffset();
	owner = segment;
}

void ElfSection::writeHeader(byte* dest)
{
	memcpy(dest,&header,sizeof(Elf32_Shdr));
}

// only called for segmentless sections
void ElfSection::writeData(ByteArray& output)
{
	if (header.sh_type == SHT_NULL) return;

	// nobits sections still get a provisional file address
	if (header.sh_type == SHT_NOBITS)
	{
		header.sh_offset = (Elf32_Off) output.size();
	}

	if (header.sh_addralign != -1)
		output.alignSize(header.sh_addralign);
	header.sh_offset = (Elf32_Off) output.size();
	output.append(data);
}

void ElfSection::setOffsetBase(int base)
{
	header.sh_offset += base;
}

ElfSegment::ElfSegment(Elf32_Phdr header, ByteArray& segmentData): header(header)
{
	data = segmentData;
	paddrSection = NULL;
}

bool ElfSegment::isSectionPartOf(ElfSection* section)
{
	int sectionStart = section->getOffset();
	int sectionSize = section->getType() == SHT_NOBITS ? 0 : section->getSize();
	int sectionEnd = sectionStart+sectionSize;

	int segmentStart = header.p_offset;
	int segmentEnd = header.p_offset+header.p_filesz;

	// exclusive > in case the size is 0
	if (sectionStart < (int)header.p_offset || sectionStart > segmentEnd) return false;

	// does an empty section belong to this or the next segment? hm...
	if (sectionStart == segmentEnd) return sectionSize == 0;

	// the start is inside the section and the size is not 0, so the end should be in here too
	if (sectionEnd > segmentEnd)
	{
		Logger::printError(Logger::Error,L"Section partially contained in segment");
		return false;
	}

	return true;
}

void ElfSegment::addSection(ElfSection* section)
{
	if (header.p_paddr != 0)
	{
		if (section->getOffset() == header.p_paddr)
		{
			paddrSection = section;
		}
	}

	section->setOwner(this);
	sections.push_back(section);
}

void ElfSegment::writeData(ByteArray& output)
{
	if (sections.size() == 0)
	{
		output.alignSize(header.p_align);
		if (header.p_offset == header.p_paddr)
			header.p_paddr = (Elf32_Addr) output.size();

		header.p_offset = (Elf32_Off) output.size();
		return;
	}

	// align segment to alignment of first section
	int align = std::max<int>(sections[0]->getAlignment(),16);
	output.alignSize(align);

	header.p_offset = (Elf32_Off) output.size();
	for (int i = 0; i < (int)sections.size(); i++)
	{
		sections[i]->setOffsetBase(header.p_offset);
	}

	if (paddrSection)
	{
		header.p_paddr = paddrSection->getOffset();
	}

	output.append(data);
}

void ElfSegment::writeHeader(byte* dest)
{
	memcpy(dest,&header,sizeof(Elf32_Phdr));
}

void ElfSegment::splitSections()
{

}

int ElfSegment::findSection(const std::string& name)
{
	for (int i = 0; i < (int)sections.size(); i++)
	{
		if (stringEqualInsensitive(name,sections[i]->getName()))
			return i;
	}

	return -1;
}

void ElfSegment::writeToData(size_t offset, void* src, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		data[offset+i] = ((byte*)src)[i];
	}
}

void ElfSegment::sortSections()
{
	std::sort(sections.begin(),sections.end(),compareSection);
}

void ElfFile::loadSectionNames()
{
	if (fileHeader.e_shstrndx == SHN_UNDEF) return;

	// check if the string table is actually a string table
	// sometimes it gives the wrong section id
	int strTablePos = sections[fileHeader.e_shstrndx]->getOffset();
	int strTableSize = sections[fileHeader.e_shstrndx]->getSize();
	for (int i = 0; i < strTableSize; i++)
	{
		if (fileData[strTablePos+i] != 0 && fileData[strTablePos+i] < 0x20)
			return;
		if (fileData[strTablePos+i] > 0x7F)
			return;
	}

	for (int i = 0; i < (int)sections.size(); i++)
	{
		ElfSection* section = sections[i];
		if (section->getType() == SHT_NULL) continue;

		int strTablePos = sections[fileHeader.e_shstrndx]->getOffset();
		int offset = strTablePos+section->getNameOffset();

		char* name = (char*) fileData.data(offset);
		std::string strName = name;
		section->setName(strName);
	}
}

void ElfFile::determinePartOrder()
{
	size_t segmentTable = fileHeader.e_phoff;
	size_t sectionTable = fileHeader.e_shoff;

	// segments
	size_t firstSegmentStart = fileData.size(), lastSegmentEnd = 0;
	for (int i = 0; i < fileHeader.e_phnum; i++)
	{
		size_t pos = fileHeader.e_phoff+i*fileHeader.e_phentsize;
		
		Elf32_Phdr segmentHeader;
		memcpy(&segmentHeader,&fileData[pos],sizeof(Elf32_Phdr));
		size_t end = segmentHeader.p_offset+segmentHeader.p_filesz;

		if ((int)segmentHeader.p_offset < firstSegmentStart) firstSegmentStart = segmentHeader.p_offset;
		if (lastSegmentEnd < end) lastSegmentEnd = end;
	}

	// segmentless sections
	size_t firstSectionStart = fileData.size(), lastSectionEnd = 0;
	for (int i = 0; i < (int)segmentlessSections.size(); i++)
	{
		if (segmentlessSections[i]->getType() == SHT_NULL) continue;

		size_t start = segmentlessSections[i]->getOffset();
		size_t end = start+segmentlessSections[i]->getSize();

		if (start == 0 && end == 0)
			continue;
		if (start < firstSectionStart) firstSectionStart = start;
		if (lastSectionEnd < end) lastSectionEnd = end;
	}

	struct PartsSort {
		size_t offset;
		ElfPart type;
		bool operator<(const PartsSort& other) const { return offset < other.offset; };
	};

	PartsSort temp[4] = {
		{ segmentTable,				ELFPART_SEGMENTTABLE },
		{ sectionTable,				ELFPART_SECTIONTABLE },
		{ firstSegmentStart,		ELFPART_SEGMENTS },
		{ firstSectionStart,		ELFPART_SEGMENTLESSSECTIONS },
	};

	std::sort(&temp[0],&temp[4]);

	for (int i = 0; i < 4; i++)
	{
		partsOrder[i] = temp[i].type;
	}
}

int ElfFile::findSegmentlessSection(const std::string& name)
{
	for (int i = 0; i < (int)sections.size(); i++)
	{
		if (stringEqualInsensitive(name,segmentlessSections[i]->getName()))
			return i;
	}

	return -1;
}

bool ElfFile::load(const std::wstring& fileName, bool sort)
{
	ByteArray data = ByteArray::fromFile(fileName);
	if (data.size() == 0)
		return false;
	return load(data,sort);
}

bool ElfFile::load(ByteArray& data, bool sort)
{
	fileData = data;

	memcpy(&fileHeader,&fileData[0],sizeof(Elf32_Ehdr));
	symTab = NULL;
	strTab = NULL;

	// load segments
	for (int i = 0; i < fileHeader.e_phnum; i++)
	{
		int pos = fileHeader.e_phoff+i*fileHeader.e_phentsize;
		
		Elf32_Phdr sectionHeader;
		memcpy(&sectionHeader,&fileData[pos],sizeof(Elf32_Phdr));

		ByteArray segmentData = fileData.mid(sectionHeader.p_offset,sectionHeader.p_filesz);
		ElfSegment* segment = new ElfSegment(sectionHeader,segmentData);
		segments.push_back(segment);
	}
	
	// load sections and assign them to segments
	for (int i = 0; i < fileHeader.e_shnum; i++)
	{
		int pos = fileHeader.e_shoff+i*fileHeader.e_shentsize;

		Elf32_Shdr sectionHeader;
		memcpy(&sectionHeader,&fileData[pos],sizeof(Elf32_Shdr));

		ElfSection* section = new ElfSection(sectionHeader);
		sections.push_back(section);

		// check if the section belongs to a segment
		ElfSegment* owner = NULL;
		for (int k = 0; k < (int)segments.size(); k++)
		{
			if (segments[k]->isSectionPartOf(section))
			{
				owner = segments[k];
				break;
			}
		}

		if (owner != NULL)
		{
			owner->addSection(section);
		} else {
			if (section->getType() != SHT_NOBITS && section->getType() != SHT_NULL)
			{
				ByteArray data = fileData.mid(section->getOffset(),section->getSize());
				section->setData(data);
			}

			switch (section->getType())
			{
			case SHT_SYMTAB:
				symTab = section;
				break;
			case SHT_STRTAB:
				strTab = section;
				break;
			}

			segmentlessSections.push_back(section);
		}
	}
	
	determinePartOrder();
	loadSectionNames();

	if (sort)
	{
		std::sort(segmentlessSections.begin(),segmentlessSections.end(),compareSection);

		for (int i = 0; i < (int)segments.size(); i++)
		{
			segments[i]->sortSections();
		}
	}

	return true;
}

void ElfFile::save(const std::wstring&fileName)
{
	fileData.clear();

	// reserve space for header and table data
	fileData.reserveBytes(sizeof(Elf32_Ehdr));

	for (int i = 0; i < 4; i++)
	{
		switch (partsOrder[i])
		{
		case ELFPART_SEGMENTTABLE:
			fileData.alignSize(4);
			fileHeader.e_phoff = (Elf32_Off) fileData.size();
			fileData.reserveBytes(segments.size()*fileHeader.e_phentsize);
			break;
		case ELFPART_SECTIONTABLE:
			fileData.alignSize(4);
			fileHeader.e_shoff = (Elf32_Off) fileData.size();
			fileData.reserveBytes(sections.size()*fileHeader.e_shentsize);
			break;
		case ELFPART_SEGMENTS:
			for (int i = 0; i < (int)segments.size(); i++)
			{
				segments[i]->writeData(fileData);
			}
			break;
		case ELFPART_SEGMENTLESSSECTIONS:
			for (int i = 0; i < (int)segmentlessSections.size(); i++)
			{
				segmentlessSections[i]->writeData(fileData);
			}
			break;
		}
	}

	// copy data to the tables
	memcpy(fileData.data(0),&fileHeader,sizeof(Elf32_Ehdr));
	for (int i = 0; i < (int)segments.size(); i++)
	{
		int pos = fileHeader.e_phoff+i*fileHeader.e_phentsize;
		segments[i]->writeHeader(fileData.data(pos));
	}
	
	for (int i = 0; i < (int)sections.size(); i++)
	{
		byte* pointer = fileData.data(fileHeader.e_shoff+i*fileHeader.e_shentsize);
		sections[i]->writeHeader(pointer);
	}

	fileData.toFile(fileName);
}

int ElfFile::getSymbolCount()
{
	if (symTab == NULL)
		return 0;

	return symTab->getSize()/sizeof(Elf32_Sym);
}

Elf32_Sym* ElfFile::getSymbol(size_t index)
{
	if (symTab == NULL)
		return NULL;
	
	return (Elf32_Sym*) &symTab->getData()[index*sizeof(Elf32_Sym)];
}

const char* ElfFile::getStrTableString(size_t pos)
{
	if (strTab == NULL)
		return NULL;
	
	return (const char*) &strTab->getData()[pos];
}