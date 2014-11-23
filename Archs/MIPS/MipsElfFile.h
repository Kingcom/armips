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
	virtual bool write(void* data, size_t length);
	virtual u64 getVirtualAddress();
	virtual u64 getPhysicalAddress();
	virtual bool seekVirtual(u64 virtualAddress);
	virtual bool seekPhysical(u64 physicalAddress);
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
	size_t sectionOffset;
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