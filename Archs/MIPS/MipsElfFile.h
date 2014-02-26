#pragma once
#include "Core/ELF/ElfFile.h"
#include "Core/FileManager.h"
#include "Commands/CAssemblerCommand.h"
#include "Util/CommonClasses.h"
#include "Mips.h"

class MipsElfFile: public AssemblerFile
{
public:
	MipsElfFile();
	virtual bool open(bool onlyCheck);
	virtual void close();
	virtual bool isOpen() { return opened; };
	virtual bool write(void* data, int length);
	virtual size_t getVirtualAddress();
	virtual size_t getPhysicalAddress();
	virtual bool seekVirtual(size_t virtualAddress);
	virtual bool seekPhysical(size_t physicalAddress);
	virtual bool getModuleInfo(SymDataModuleInfo& info);

	bool load(const std::wstring& fileName, const std::wstring& outputFileName);
	void save();
	bool setSection(const std::wstring& name);
private:
	ElfFile elf;
	std::wstring fileName;
	std::wstring outputFileName;
	bool opened;
	int platform;

	int segment;
	int section;
	int sectionOffset;
};


class DirectiveLoadMipsElf: public CAssemblerCommand
{
public:
	DirectiveLoadMipsElf(ArgumentList& args);
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
	virtual void writeSymData(SymbolData& symData) { };
private:
	MipsElfFile* file;
	std::wstring inputName;
	std::wstring outputName;
};