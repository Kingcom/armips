#pragma once

#include "Util/ByteArray.h"
#include "ElfTypes.h"
#include <vector>

enum ElfPart { ELFPART_SEGMENTTABLE, ELFPART_SECTIONTABLE, ELFPART_SEGMENTS, ELFPART_SEGMENTLESSSECTIONS }; 

class ElfSegment;
class ElfSection;

class ElfFile
{
public:

	bool load(const std::wstring&fileName, bool sort);
	void save(const std::wstring&fileName);

	Elf32_Half getType() { return fileHeader.e_type; };
	int getSegmentCount() { return segments.size(); };
	ElfSegment* getSegment(int index) { return segments[index]; };

	int findSegmentlessSection(const std::string& name);
	ElfSection* getSegmentlessSection(int index) { return segmentlessSections[index]; };
	int getSegmentlessSectionCount() { return segmentlessSections.size(); };
	ByteArray& getFileData() { return fileData; }

	int getSymbolCount();
	Elf32_Sym* getSymbol(int index);
	const char* getStrTableString(int pos);
private:
	void loadSectionNames();
	void determinePartOrder();

	Elf32_Ehdr fileHeader;
	std::vector<ElfSegment*> segments;
	std::vector<ElfSection*> sections;
	std::vector<ElfSection*> segmentlessSections;
	ByteArray fileData;
	ElfPart partsOrder[4];

	ElfSection* symTab;
	ElfSection* strTab;
};


class ElfSection
{
public:
	ElfSection(Elf32_Shdr header);
	void setName(std::string& name) { this->name = name; };
	const std::string& getName() { return name; };
	void setData(ByteArray& data) { this->data = data; };
	void setOwner(ElfSegment* segment);
	bool hasOwner() { return owner != NULL; };
	void writeHeader(byte* dest);
	void writeData(ByteArray& output);
	void setOffsetBase(int base);
	ByteArray& getData() { return data; };
	
	Elf32_Word getType() { return header.sh_type; };
	Elf32_Off getOffset() { return header.sh_offset; };
	Elf32_Word getSize() { return header.sh_size; };
	Elf32_Word getNameOffset() { return header.sh_name; };
	Elf32_Word getAlignment() { return header.sh_addralign; };
	Elf32_Addr getAddress() { return header.sh_addr; };
	Elf32_Half getInfo() { return header.sh_info; };
	Elf32_Word getFlags() { return header.sh_flags; };
private:
	Elf32_Shdr header;
	std::string name;
	ByteArray data;
	ElfSegment* owner;
};

class ElfSegment
{
public:
	ElfSegment(Elf32_Phdr header, ByteArray& segmentData);
	bool isSectionPartOf(ElfSection* section);
	void addSection(ElfSection* section);
	Elf32_Off getOffset() { return header.p_offset; };
	Elf32_Word getPhysSize() { return header.p_filesz; };
	Elf32_Word getType() { return header.p_type; };
	Elf32_Addr getVirtualAddress() { return header.p_vaddr; };
	int getSectionCount() { return sections.size(); };
	void writeHeader(byte* dest);
	void writeData(ByteArray& output);
	void splitSections();

	int findSection(const std::string& name);
	ElfSection* getSection(int index) { return sections[index]; };
	void writeToData(int offset, void* data, int size);
	void sortSections();
private:
	Elf32_Phdr header;
	ByteArray data;
	std::vector<ElfSection*> sections;
	ElfSection* paddrSection;
};

struct RelocationData
{
	unsigned int opcodeOffset;
	unsigned int opcode;
	unsigned int relocationBase;

	unsigned int symbolAddress;
	int targetSymbolType;
	int targetSymbolInfo;
};

class IElfRelocator
{
public:
	virtual ~IElfRelocator() { };
	virtual bool relocateOpcode(int type, RelocationData& data) = 0;
	virtual void setSymbolAddress(RelocationData& data, unsigned int symbolAddress, int symbolType) = 0;
};
