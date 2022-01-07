#pragma once

#include "Commands/CAssemblerCommand.h"
#include "Core/ELF/ElfFile.h"
#include "Core/FileManager.h"

class MipsElfFile: public AssemblerFile
{
public:
	MipsElfFile();
	virtual bool open(bool onlyCheck);
	virtual void close();
	virtual bool isOpen() { return opened; };
	virtual bool write(void* data, size_t length);
	virtual int64_t getVirtualAddress();
	virtual int64_t getPhysicalAddress();
	virtual int64_t getHeaderSize();
	virtual bool seekVirtual(int64_t virtualAddress);
	virtual bool seekPhysical(int64_t physicalAddress);
	virtual bool getModuleInfo(SymDataModuleInfo& info);
	virtual void beginSymData(SymbolData& symData);
	virtual void endSymData(SymbolData& symData);
	virtual const fs::path& getFileName() { return fileName; };

	bool load(const fs::path& fileName, const fs::path& outputFileName);
	void save();
	bool setSection(const std::string& name);
private:
	ElfFile elf;
	fs::path fileName;
	fs::path outputFileName;
	bool opened;
	int platform;

	int segment;
	int section;
	size_t sectionOffset;
};


class DirectiveLoadMipsElf: public CAssemblerCommand
{
public:
	DirectiveLoadMipsElf(const fs::path& fileName);
	DirectiveLoadMipsElf(const fs::path& inputName, const fs::path& outputName);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
private:
	std::shared_ptr<MipsElfFile> file;
	fs::path inputName;
	fs::path outputName;
};
