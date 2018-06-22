#pragma once
#include "ElfFile.h"
#include "Core/SymbolData.h"

struct ElfRelocatorCtor
{
	std::wstring symbolName;
	size_t size;
};

class CAssemblerCommand;
class Parser;

class IElfRelocator
{
public:
	virtual ~IElfRelocator() {};
	virtual int expectedMachine() const = 0;
	virtual bool isDummyRelocationType(int type) const { return false; }
	virtual bool relocateOpcode(int type, RelocationData& data) = 0;
	virtual void setSymbolAddress(RelocationData& data, int64_t symbolAddress, int symbolType) = 0;

	virtual std::unique_ptr<CAssemblerCommand> generateCtorStub(std::vector<ElfRelocatorCtor>& ctors) { return nullptr; }
};


class Label;

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
	std::wstring name;
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
	std::wstring name;
};

class ElfRelocator
{
public:
	bool init(const std::wstring& inputName);
	bool exportSymbols();
	void writeSymbols(SymbolData& symData) const;
	std::unique_ptr<CAssemblerCommand> generateCtor(const std::wstring& ctorName);
	bool relocate(int64_t& memoryAddress);
	bool hasDataChanged() { return dataChanged; };
	const ByteArray& getData() const { return outputData; };
private:
	bool relocateFile(ElfRelocatorFile& file, int64_t& relocationAddress);
	void loadRelocation(Elf32_Rel& rel, ByteArray& data, int offset, Endianness endianness);

	ByteArray outputData;
	std::unique_ptr<IElfRelocator> relocator;
	std::vector<ElfRelocatorFile> files;
	std::vector<ElfRelocatorCtor> ctors;
	bool dataChanged;
};
