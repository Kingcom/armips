#pragma once
#include "Archs/MIPS/ELF/ElfFile.h"
#include "Core/FileManager.h"
#include "Commands/CAssemblerCommand.h"
#include "Util/CommonClasses.h"

class PspElfFile: public AssemblerFile
{
public:
	PspElfFile();
	virtual bool open(bool onlyCheck);
	virtual void close();
	virtual bool isOpen() { return opened; };
	virtual bool write(void* data, int length);
	virtual size_t getVirtualAddress();
	virtual size_t getPhysicalAddress();
	virtual bool seekVirtual(size_t virtualAddress);
	virtual bool seekPhysical(size_t physicalAddress);

	bool load(const std::wstring& fileName, const std::wstring& outputFileName);
	void save();
	bool setSection(const std::wstring& name);
private:
	ElfFile elf;
	std::wstring fileName;
	std::wstring outputFileName;
	bool opened;

	int segment;
	int section;
	int sectionOffset;
};


class DirectiveLoadPspElf: public CAssemblerCommand
{
public:
	DirectiveLoadPspElf(ArgumentList& args);
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
	virtual void writeSymData(SymbolData& symData) { };
private:
	PspElfFile* file;
	std::wstring inputName;
	std::wstring outputName;
};