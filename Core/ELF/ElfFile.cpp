#include "Core/ELF/ElfFile.h"

#include "Core/Misc.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <vector>

static bool stringEqualInsensitive(const std::string& a, const std::string& b)
{
	if (a.size() != b.size())
		return false;

	auto compare = [](char c1, char c2)
	{
		return std::tolower(c1) == std::tolower(c2);
	};

	return std::equal(a.begin(), a.end(), b.begin(), compare);
}

bool compareSection(ElfSection* a, ElfSection* b)
{
	return a->getOffset() < b->getOffset();
}

ElfSection::ElfSection(Elf32_Shdr header): header(header)
{
	owner = nullptr;
}

void ElfSection::setOwner(ElfSegment* segment)
{
	header.sh_offset -= segment->getOffset();
	owner = segment;
}

void ElfSection::writeHeader(ByteArray& data, size_t pos, Endianness endianness)
{
	data.replaceDoubleWord(pos + 0x00, header.sh_name, endianness);
	data.replaceDoubleWord(pos + 0x04, header.sh_type, endianness);
	data.replaceDoubleWord(pos + 0x08, header.sh_flags, endianness);
	data.replaceDoubleWord(pos + 0x0C, header.sh_addr, endianness);
	data.replaceDoubleWord(pos + 0x10, header.sh_offset, endianness);
	data.replaceDoubleWord(pos + 0x14, header.sh_size, endianness);
	data.replaceDoubleWord(pos + 0x18, header.sh_link, endianness);
	data.replaceDoubleWord(pos + 0x1C, header.sh_info, endianness);
	data.replaceDoubleWord(pos + 0x20, header.sh_addralign, endianness);
	data.replaceDoubleWord(pos + 0x24, header.sh_entsize, endianness);
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

	if (header.sh_addralign != (unsigned) -1)
		output.alignSize(header.sh_addralign);
	header.sh_offset = (Elf32_Off) output.size();
	output.append(data);
}

void ElfSection::setOffsetBase(int base)
{
	header.sh_offset += base;
}

ElfSegment::ElfSegment(Elf32_Phdr header, ByteArray& segmentData)
	: header(header), data(segmentData), paddrSection(nullptr)
{
}

bool ElfSegment::isSectionPartOf(ElfSection* section)
{
	int sectionStart = section->getOffset();
	int sectionSize = section->getType() == SHT_NOBITS ? 0 : section->getSize();
	int sectionEnd = sectionStart+sectionSize;

	int segmentStart = header.p_offset;
	int segmentEnd = segmentStart+header.p_filesz;

	// exclusive > in case the size is 0
	if (sectionStart < (int)header.p_offset || sectionStart > segmentEnd) return false;

	// does an empty section belong to this or the next segment? hm...
	if (sectionStart == segmentEnd) return sectionSize == 0;

	// the start is inside the section and the size is not 0, so the end should be in here too
	if (sectionEnd > segmentEnd)
	{
		Logger::printError(Logger::Error, "Section partially contained in segment");
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

void ElfSegment::writeHeader(ByteArray& data, size_t pos, Endianness endianness)
{
	data.replaceDoubleWord(pos + 0x00, header.p_type, endianness);
	data.replaceDoubleWord(pos + 0x04, header.p_offset, endianness);
	data.replaceDoubleWord(pos + 0x08, header.p_vaddr, endianness);
	data.replaceDoubleWord(pos + 0x0C, header.p_paddr, endianness);
	data.replaceDoubleWord(pos + 0x10, header.p_filesz, endianness);
	data.replaceDoubleWord(pos + 0x14, header.p_memsz, endianness);
	data.replaceDoubleWord(pos + 0x18, header.p_flags, endianness);
	data.replaceDoubleWord(pos + 0x1C, header.p_align, endianness);
}

void ElfSegment::splitSections()
{

}

int ElfSegment::findSection(const std::string& name)
{
	for (size_t i = 0; i < sections.size(); i++)
	{
		if (stringEqualInsensitive(name,sections[i]->getName()))
			return (int)i;
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
	size_t strTablePos = sections[fileHeader.e_shstrndx]->getOffset();
	size_t strTableSize = sections[fileHeader.e_shstrndx]->getSize();
	for (size_t i = 0; i < strTableSize; i++)
	{
		if (fileData[strTablePos+i] != 0 && fileData[strTablePos+i] < 0x20)
			return;
		if (fileData[strTablePos+i] > 0x7F)
			return;
	}

	for (size_t i = 0; i < sections.size(); i++)
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
	for (size_t i = 0; i < fileHeader.e_phnum; i++)
	{
		size_t pos = fileHeader.e_phoff+i*fileHeader.e_phentsize;
		
		Elf32_Phdr segmentHeader;
		loadProgramHeader(segmentHeader, fileData, pos);
		size_t end = segmentHeader.p_offset + segmentHeader.p_filesz;

		if (segmentHeader.p_offset < firstSegmentStart) firstSegmentStart = segmentHeader.p_offset;
		if (lastSegmentEnd < end) lastSegmentEnd = end;
	}

	// segmentless sections
	size_t firstSectionStart = fileData.size(), lastSectionEnd = 0;
	for (size_t i = 0; i < segmentlessSections.size(); i++)
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

	for (size_t i = 0; i < 4; i++)
	{
		partsOrder[i] = temp[i].type;
	}
}

int ElfFile::findSegmentlessSection(const std::string& name)
{
	for (size_t i = 0; i < segmentlessSections.size(); i++)
	{
		if (stringEqualInsensitive(name,segmentlessSections[i]->getName()))
			return (int)i;
	}

	return -1;
}

void ElfFile::loadElfHeader()
{
	memcpy(fileHeader.e_ident, &fileData[0], sizeof(fileHeader.e_ident));
	Endianness endianness = getEndianness();
	fileHeader.e_type = fileData.getWord(0x10, endianness);
	fileHeader.e_machine = fileData.getWord(0x12, endianness);
	fileHeader.e_version = fileData.getDoubleWord(0x14, endianness);
	fileHeader.e_entry = fileData.getDoubleWord(0x18, endianness);
	fileHeader.e_phoff = fileData.getDoubleWord(0x1C, endianness);
	fileHeader.e_shoff = fileData.getDoubleWord(0x20, endianness);
	fileHeader.e_flags = fileData.getDoubleWord(0x24, endianness);
	fileHeader.e_ehsize = fileData.getWord(0x28, endianness);
	fileHeader.e_phentsize = fileData.getWord(0x2A, endianness);
	fileHeader.e_phnum = fileData.getWord(0x2C, endianness);
	fileHeader.e_shentsize = fileData.getWord(0x2E, endianness);
	fileHeader.e_shnum = fileData.getWord(0x30, endianness);
	fileHeader.e_shstrndx = fileData.getWord(0x32, endianness);
}

void ElfFile::writeHeader(ByteArray& data, size_t pos, Endianness endianness)
{
	memcpy(&fileData[0], fileHeader.e_ident, sizeof(fileHeader.e_ident));
	data.replaceWord(pos + 0x10, fileHeader.e_type, endianness);
	data.replaceWord(pos + 0x12, fileHeader.e_machine, endianness);
	data.replaceDoubleWord(pos + 0x14, fileHeader.e_version, endianness);
	data.replaceDoubleWord(pos + 0x18, fileHeader.e_entry, endianness);
	data.replaceDoubleWord(pos + 0x1C, fileHeader.e_phoff, endianness);
	data.replaceDoubleWord(pos + 0x20, fileHeader.e_shoff, endianness);
	data.replaceDoubleWord(pos + 0x24, fileHeader.e_flags, endianness);
	data.replaceWord(pos + 0x28, fileHeader.e_ehsize, endianness);
	data.replaceWord(pos + 0x2A, fileHeader.e_phentsize, endianness);
	data.replaceWord(pos + 0x2C, fileHeader.e_phnum, endianness);
	data.replaceWord(pos + 0x2E, fileHeader.e_shentsize, endianness);
	data.replaceWord(pos + 0x30, fileHeader.e_shnum, endianness);
	data.replaceWord(pos + 0x32, fileHeader.e_shstrndx, endianness);
}

void ElfFile::loadProgramHeader(Elf32_Phdr& header, ByteArray& data, size_t pos)
{
	Endianness endianness = getEndianness();
	header.p_type   = data.getDoubleWord(pos + 0x00, endianness);
	header.p_offset = data.getDoubleWord(pos + 0x04, endianness);
	header.p_vaddr  = data.getDoubleWord(pos + 0x08, endianness);
	header.p_paddr  = data.getDoubleWord(pos + 0x0C, endianness);
	header.p_filesz = data.getDoubleWord(pos + 0x10, endianness);
	header.p_memsz  = data.getDoubleWord(pos + 0x14, endianness);
	header.p_flags  = data.getDoubleWord(pos + 0x18, endianness);
	header.p_align  = data.getDoubleWord(pos + 0x1C, endianness);
}

void ElfFile::loadSectionHeader(Elf32_Shdr& header, ByteArray& data, size_t pos)
{
	Endianness endianness = getEndianness();
	header.sh_name      = data.getDoubleWord(pos + 0x00, endianness);
	header.sh_type      = data.getDoubleWord(pos + 0x04, endianness);
	header.sh_flags     = data.getDoubleWord(pos + 0x08, endianness);
	header.sh_addr      = data.getDoubleWord(pos + 0x0C, endianness);
	header.sh_offset    = data.getDoubleWord(pos + 0x10, endianness);
	header.sh_size      = data.getDoubleWord(pos + 0x14, endianness);
	header.sh_link      = data.getDoubleWord(pos + 0x18, endianness);
	header.sh_info      = data.getDoubleWord(pos + 0x1C, endianness);
	header.sh_addralign = data.getDoubleWord(pos + 0x20, endianness);
	header.sh_entsize   = data.getDoubleWord(pos + 0x24, endianness);
}

bool ElfFile::load(const fs::path& fileName, bool sort)
{
	ByteArray data = ByteArray::fromFile(fileName);
	if (data.size() == 0)
		return false;
	return load(data,sort);
}

bool ElfFile::load(ByteArray& data, bool sort)
{
	fileData = data;

	loadElfHeader();
	symTab = nullptr;
	strTab = nullptr;

	// load segments
	for (size_t i = 0; i < fileHeader.e_phnum; i++)
	{
		size_t pos = fileHeader.e_phoff+i*fileHeader.e_phentsize;
		
		Elf32_Phdr sectionHeader;
		loadProgramHeader(sectionHeader, fileData, pos);

		ByteArray segmentData = fileData.mid(sectionHeader.p_offset,sectionHeader.p_filesz);
		ElfSegment* segment = new ElfSegment(sectionHeader,segmentData);
		segments.push_back(segment);
	}
	
	// load sections and assign them to segments
	for (int i = 0; i < fileHeader.e_shnum; i++)
	{
		size_t pos = fileHeader.e_shoff+i*fileHeader.e_shentsize;

		Elf32_Shdr sectionHeader;
		loadSectionHeader(sectionHeader, fileData, pos);

		ElfSection* section = new ElfSection(sectionHeader);
		sections.push_back(section);

		// check if the section belongs to a segment
		ElfSegment* owner = nullptr;
		for (int k = 0; k < (int)segments.size(); k++)
		{
			if (segments[k]->isSectionPartOf(section))
			{
				owner = segments[k];
				break;
			}
		}

		if (owner != nullptr)
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
				if (!strTab || i != fileHeader.e_shstrndx)
				{
					strTab = section;
				}
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

void ElfFile::save(const fs::path& fileName)
{
	fileData.clear();

	// reserve space for header and table data
	fileData.reserveBytes(sizeof(Elf32_Ehdr));

	for (size_t i = 0; i < 4; i++)
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
			for (size_t i = 0; i < segments.size(); i++)
			{
				segments[i]->writeData(fileData);
			}
			break;
		case ELFPART_SEGMENTLESSSECTIONS:
			for (size_t i = 0; i < segmentlessSections.size(); i++)
			{
				segmentlessSections[i]->writeData(fileData);
			}
			break;
		}
	}

	// copy data to the tables
	Endianness endianness = getEndianness();
	writeHeader(fileData, 0, endianness);
	for (size_t i = 0; i < segments.size(); i++)
	{
		size_t pos = fileHeader.e_phoff+i*fileHeader.e_phentsize;
		segments[i]->writeHeader(fileData, pos, endianness);
	}
	
	for (size_t i = 0; i < sections.size(); i++)
	{
		size_t pos = fileHeader.e_shoff+i*fileHeader.e_shentsize;
		sections[i]->writeHeader(fileData, pos, endianness);
	}

	fileData.toFile(fileName);
}

int ElfFile::getSymbolCount()
{
	if (symTab == nullptr)
		return 0;

	return symTab->getSize()/sizeof(Elf32_Sym);
}

bool ElfFile::getSymbol(Elf32_Sym& symbol, size_t index)
{
	if (symTab == nullptr)
		return false;

	ByteArray &data = symTab->getData();
	size_t pos = index*sizeof(Elf32_Sym);
	Endianness endianness = getEndianness();
	symbol.st_name  = data.getDoubleWord(pos + 0x00, endianness);
	symbol.st_value = data.getDoubleWord(pos + 0x04, endianness);
	symbol.st_size  = data.getDoubleWord(pos + 0x08, endianness);
	symbol.st_info  = data[pos + 0x0C];
	symbol.st_other = data[pos + 0x0D];
	symbol.st_shndx = data.getWord(pos + 0x0E, endianness);

	return true;
}

const char* ElfFile::getStrTableString(size_t pos)
{
	if (strTab == nullptr)
		return nullptr;
	
	return (const char*) &strTab->getData()[pos];
}
