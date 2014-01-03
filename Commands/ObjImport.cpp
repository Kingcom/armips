#include "stdafx.h"
#include "ObjImport.h"
#include "Core/ELF/ElfFile.h"
#include "Core/Common.h"
#include "Core/Misc.h"
#include "Core/MathParser.h"
#include "Core/FileManager.h"
#include <map>

std::wstring toWLowercase(const std::string& str)
{
	std::wstring result;
	for (size_t i = 0; i < str.size(); i++)
	{
		result += tolower(str[i]);
	}

	return result;
}

DirectiveObjImport::DirectiveObjImport(ArgumentList& args)
{
	inputName = args[0].text;
	relocator = NULL;
}

DirectiveObjImport::~DirectiveObjImport()
{
	if (relocator != NULL)
		delete relocator;
}

bool DirectiveObjImport::init()
{
	relocator = Arch->getElfRelocator();
	if (relocator == NULL)
	{
		Logger::printError(Logger::Error,L"Object importing not supported for this architecture");
		return false;
	}

	if (elf.load(inputName,false) == false)
	{
		Logger::printError(Logger::Error,L"Could not load object file");
		return false;
	}

	if (elf.getType() != 1)
	{
		Logger::printError(Logger::Error,L"Unexpected elf type");
		return false;
	}

	if (elf.getSegmentCount() != 0)
	{
		Logger::printError(Logger::Error,L"Unexpected segment count");
		return false;
	}

	// init symbols

	// now load symbols
	for (int i = 0; i < elf.getSymbolCount(); i++)
	{
		Elf32_Sym* symbol = elf.getSymbol(i);
		std::wstring name = toWLowercase(elf.getStrTableString(symbol->st_name));

		switch (symbol->st_info & 0xF)
		{
		case STT_OBJECT:
		case STT_FUNC:
			{
				Label* label = Global.symbolTable.getLabel(name,-1,-1);
				if (label == NULL)
				{
					Logger::printError(Logger::Error,L"Invalid label name \"%s\"",name.c_str());
					continue;
				}

				if (label->isDefined())
				{
					Logger::printError(Logger::Error,L"Label \"%s\" already defined",name.c_str());
					continue;
				}

				label->setValue(0);
				label->setDefined(true);
			}
			break;
		}
	}

	return true;
}

bool DirectiveObjImport::loadData(ElfFile& elf)
{
	fileData.clear();
	symbols.clear();

	std::map<int,ElfSection*> sections;
	std::map<int,int> relocationOffsets;

	int totalSize = 0;
	int relocationAddress = g_fileManager->getVirtualAddress();
	int start = relocationAddress;

	for (int i = 0; i < elf.getSegmentlessSectionCount(); i++)
	{
		ElfSection* section = elf.getSegmentlessSection(i);
		if (!(section->getFlags() & SHF_ALLOC))
			continue;

		int curSize = section->getSize();
		switch (section->getType())
		{
		case SHT_PROGBITS:
		case SHT_NOBITS:
			sections[i] = section;
			relocationOffsets[i] = relocationAddress;
			relocationAddress += curSize;
			totalSize += curSize;
			break;
		}
	}

	fileData.resize(totalSize);

	for (auto it = sections.begin(); it != sections.end(); it++)
	{
		ElfSection* section = it->second;
		int index = it->first;

		if (section->getType() == SHT_NOBITS)
		{
			int arrayStart = relocationOffsets[index]-start;
			for (size_t i = 0; i < section->getSize(); i++)
			{
				fileData[arrayStart+i] = 0;
			}
			continue;
		}

		ByteArray sectionData = section->getData();

		// look for relocation sections
		for (int s = 0; s < elf.getSegmentlessSectionCount(); s++)
		{
			ElfSection* relSection = elf.getSegmentlessSection(s);
			if (relSection->getType() != SHT_REL)
				continue;
			if (relSection->getInfo() != index)
				continue;

			// got it!
			Elf32_Rel* rel = (Elf32_Rel*) &relSection->getData()[0];
			int relCount = relSection->getSize()/sizeof(Elf32_Rel);

			for (int i = 0; i < relCount; i++)
			{
				int pos = rel[i].r_offset;

				int symNum = rel[i].getSymbolNum();
				if (symNum <= 0)
				{
					Logger::queueError(Logger::Warning,L"Invalid symbol num %06X",symNum);
					continue;
				}


				auto sym = elf.getSymbol(symNum);
				int symSection = sym->st_shndx;
				
				RelocationData relData;
				relData.opcode = sectionData.getDoubleWord(pos);
				relData.opcodeOffset = pos+relocationOffsets[index];
				relocator->setSymbolAddress(relData,sym->st_value,sym->st_info & 0xF);

				// externs?
				if (relData.targetSymbolType == STT_NOTYPE && sym->st_shndx == 0)
				{
					std::wstring symName = toWLowercase(elf.getStrTableString(sym->st_name));

					Label* label = Global.symbolTable.getLabel(symName,-1,-1);
					if (label == NULL)
					{
						Logger::queueError(Logger::Error,L"Invalid external symbol %s",symName.c_str());
						continue;
					}
					if (label->isDefined() == false)
					{
						Logger::queueError(Logger::Error,L"Undefined external symbol %s",symName.c_str());
						continue;
					}

					relData.relocationBase = label->getValue();
					relData.targetSymbolType = label->isData() ? STT_OBJECT : STT_FUNC;
					relData.targetSymbolInfo = label->getInfo();
				} else {
					relData.relocationBase = relocationOffsets[symSection]+relData.symbolAddress;
				}

				if (relocator->relocateOpcode(rel[i].getType(),relData) == false)
				{
					Logger::queueError(Logger::Error,relData.errorMessage);
					continue;
				}

				sectionData.replaceDoubleWord(pos,relData.opcode);
			}

			break;
		}

		// insert data into array
		int arrayStart = relocationOffsets[index]-start;
		for (size_t i = 0; i < section->getSize(); i++)
		{
			fileData[arrayStart+i] = sectionData[i];
		}
	}

	// now load symbols
	for (int i = 0; i < elf.getSymbolCount(); i++)
	{
		SymbolEntry entry;

		Elf32_Sym* symbol = elf.getSymbol(i);
		entry.name = toWLowercase(elf.getStrTableString(symbol->st_name));
		entry.address = symbol->st_value+relocationOffsets[symbol->st_shndx];
		entry.size = symbol->st_size;
		entry.type = symbol->st_info & 0xF;

		switch (symbol->st_info & 0xF)
		{
		case STT_OBJECT:
		case STT_FUNC:
			symbols.push_back(entry);
			break;
		}
	}

	return true;
}


bool DirectiveObjImport::Validate()
{
	if (loadData(elf) == false)
	{
		Logger::queueError(Logger::Error,L"Failed to relocate object data");
		return false;
	}
	
	for (size_t i = 0; i < symbols.size(); i++)
	{
		Label* label = Global.symbolTable.getLabel(symbols[i].name,-1,-1);
		label->setValue(symbols[i].address);
	}

	g_fileManager->advanceMemory(fileData.size());
	return false;
}

void DirectiveObjImport::Encode()
{
	g_fileManager->write(fileData.data(),fileData.size());
}

void DirectiveObjImport::writeSymData(SymbolData& symData)
{
	for (size_t i = 0; i < symbols.size(); i++)
	{
		symData.addLabel(symbols[i].address,symbols[i].name.c_str());

		switch (symbols[i].type)
		{
		case STT_OBJECT:
			symData.addData(symbols[i].address,symbols[i].size,SymbolData::Data8);
			break;
		case STT_FUNC:
			Global.symData.startFunction(symbols[i].address);
			Global.symData.endFunction(symbols[i].address+symbols[i].size);
			break;
		}
	}
}