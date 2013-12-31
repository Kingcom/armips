#pragma once

#include "Commands/CAssemblerCommand.h"
#include "Util/CommonClasses.h"
#include "ElfFile.h"

class DirectivePspObjImport: public CAssemblerCommand
{
public:
	DirectivePspObjImport(ArgumentList& args);
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
	ByteArray fileData;
	std::vector<SymbolEntry> symbols;
	std::wstring inputName;
};