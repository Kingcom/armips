#pragma once
#include "ElfFile.h"
#include "Core/SymbolData.h"

struct ElfRelocatorCtor
{
	std::wstring symbolName;
	size_t size;
};

class IElfRelocator
{
public:
	virtual ~IElfRelocator() { };
	virtual bool relocateOpcode(int type, RelocationData& data) = 0;
	virtual void setSymbolAddress(RelocationData& data, u64 symbolAddress, int symbolType) = 0;
	virtual void writeCtorStub(std::vector<ElfRelocatorCtor>& ctors) = 0;
};

class Label;

struct ElfRelocatorSection
{
	ElfSection* section;
	size_t index;
	ElfSection* relSection;
	Label* label;
};

struct ElfRelocatorSymbol
{
	Label* label;
	std::wstring name;
	u64 relativeAddress;
	u64 relocatedAddress;
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
	void writeSymbols(SymbolData& symData);
	void writeCtor(const std::wstring& ctorName);
	bool relocate(u64& memoryAddress);
	bool hasDataChanged() { return dataChanged; };
	ByteArray& getData() { return outputData; };
private:
	bool relocateFile(ElfRelocatorFile& file, u64& relocationAddress);

	ByteArray outputData;
	IElfRelocator* relocator;
	std::vector<ElfRelocatorFile> files;
	std::vector<ElfRelocatorCtor> ctors;
	bool dataChanged;
};