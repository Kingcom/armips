#pragma once

#include "Commands/CAssemblerCommand.h"
#include "Util/CommonClasses.h"
#include "Core/ELF/ElfFile.h"

class IELfRelocator;

class DirectiveObjImport: public CAssemblerCommand
{
public:
	DirectiveObjImport(ArgumentList& args);
	~DirectiveObjImport();
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData) { };
	virtual void writeSymData(SymbolData& symData);

	bool init();
private:
	struct SymbolEntry
	{
		int type;
		std::wstring name;
		unsigned int address;
		unsigned int size;
	};

	bool loadData(ElfFile& elf);

	ElfFile elf;
	IElfRelocator* relocator;
	ByteArray fileData;
	std::vector<SymbolEntry> symbols;
	std::wstring inputName;
};