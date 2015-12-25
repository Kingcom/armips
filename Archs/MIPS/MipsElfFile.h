#pragma once
#include "Core/ELF/ElfFile.h"
#include "Core/FileManager.h"
#include "Commands/CAssemblerCommand.h"
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
	virtual void beginSymData();
	virtual void endSymData();

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
	DirectiveLoadMipsElf(const std::wstring& fileName);
	DirectiveLoadMipsElf(const std::wstring& inputName, const std::wstring& outputName);
	virtual bool Validate();
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
	virtual void writeSymData(SymbolData& symData) const;
private:
	MipsElfFile* file;
	std::wstring inputName;
	std::wstring outputName;
};