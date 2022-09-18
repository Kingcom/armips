#pragma once

#include "Core/ELF/ElfFile.h"
#include "Core/Types.h"

#include <memory>

struct ElfRelocatorCtor
{
	Identifier symbolName;
	size_t size;
};

struct RelocationAction
{
	RelocationAction(int64_t offset, uint32_t newValue) : offset(offset), newValue(newValue) {}
	int64_t offset;
	uint32_t newValue;
};

class CAssemblerCommand;
class Identifier;
class Parser;

class IElfRelocator
{
public:
	virtual ~IElfRelocator() {};
	virtual int expectedMachine() const = 0;
	virtual bool isDummyRelocationType(int type) const { return false; }
	virtual bool relocateOpcode(int type, const RelocationData& data, std::vector<RelocationAction>& actions, std::vector<std::string>& errors) = 0;
	virtual bool finish(std::vector<RelocationAction>& actions, std::vector<std::string>& errors) { return true; }
	virtual void setSymbolAddress(RelocationData& data, int64_t symbolAddress, int symbolType) = 0;

	virtual std::unique_ptr<CAssemblerCommand> generateCtorStub(std::vector<ElfRelocatorCtor>& ctors);
};


class Label;
class SymbolData;

struct ElfRelocatorSection
{
	ElfSection* section;
	size_t index;
	ElfSection* relSection;
	std::shared_ptr<Label> label;
};

struct ElfRelocatorSymbol
{
	std::shared_ptr<Label> label;
	std::string name;
	int64_t relativeAddress;
	int64_t relocatedAddress;
	size_t section;
	size_t size;
	int type;
};

struct ElfRelocatorFile
{
	ElfFile* elf;
	std::vector<ElfRelocatorSection> sections;
	std::vector<ElfRelocatorSymbol> symbols;
	std::string name;
};

class ElfRelocator
{
public:
	bool init(const fs::path& inputName);
	bool exportSymbols();
	void writeSymbols(SymbolData& symData) const;
	std::unique_ptr<CAssemblerCommand> generateCtor(const Identifier& ctorName);
	bool relocate(int64_t& memoryAddress);
	bool hasDataChanged() { return dataChanged; };
	const ByteArray& getData() const { return outputData; };
private:
	bool relocateFile(ElfRelocatorFile& file, int64_t& relocationAddress);
	void loadRelocation(Elf32_Rela& rela, bool addend, ByteArray& data, int offset, Endianness endianness);

	ByteArray outputData;
	std::unique_ptr<IElfRelocator> relocator;
	std::vector<ElfRelocatorFile> files;
	std::vector<ElfRelocatorCtor> ctors;
	bool dataChanged;
};
