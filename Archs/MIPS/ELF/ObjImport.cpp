#include "stdafx.h"
#include "ObjImport.h"
#include "ElfFile.h"
#include "Core/Common.h"
#include "Core/Misc.h"
#include "Core/MathParser.h"
#include "Core/FileManager.h"
#include <map>

DirectivePspObjImport::DirectivePspObjImport(ArgumentList& args)
{
	inputName = args[0].text;
}

bool DirectivePspObjImport::init()
{
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

	return true;
}

bool DirectivePspObjImport::loadData(ElfFile& elf)
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
				unsigned int op = sectionData.getDoubleWord(pos);

				int symNum = rel[i].getSymbolNum();
				if (symNum <= 0)
				{
					Logger::queueError(Logger::Warning,L"Invalid symbol num %06X",symNum);
					continue;
				}

				auto sym = elf.getSymbol(symNum);
				int symSection = sym->st_shndx;
				int relTo;

				// externs?
				if ((sym->st_info & 0xF) == STT_NOTYPE)
				{
					std::string symName = elf.getStrTableString(sym->st_name);
					for (size_t k = 0; k < symName.size(); k++)
					{
						symName[k] = tolower(symName[k]);
					}

					Label* label = Global.symbolTable.getLabel(convertUtf8ToWString(symName.c_str()),-1,-1);
					if (label == NULL)
					{
						Logger::queueError(Logger::Error,L"Invalid external symbol %S",symName);
						continue;
					}
					if (label->isDefined() == false)
					{
						Logger::queueError(Logger::Error,L"Undefined external symbol %S",symName);
						continue;
					}

					relTo = relocationOffsets[symSection]+label->getValue();
				} else {
					relTo = relocationOffsets[symSection]+sym->st_value;
				}

				unsigned int p;
				switch (rel[i].getType())
				{
				case R_MIPS_26: //j, jal
					op = (op & 0xFC000000) | (((op&0x03FFFFFF)+(relTo>>2))&0x03FFFFFF);
					break;
				case R_MIPS_32:
					op += relTo;
					break;
				case R_MIPS_HI16:
					p = (op & 0xFFFF) + relTo;
					op = (op&0xffff0000) | (((p >> 16) + ((p & 0x8000) != 0)) & 0xFFFF);
					break;
				case R_MIPS_LO16:
					op = (op&0xffff0000) | (((op&0xffff)+relTo)&0xffff);
					break;
				default:
					Logger::queueError(Logger::Error,L"Unknown relocation type %d\n",rel[i].getType());
					break;
				}

				sectionData.replaceDoubleWord(pos,op);
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
		entry.name = elf.getStrTableString(symbol->st_name);
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


bool DirectivePspObjImport::Validate()
{
	if (loadData(elf) == false)
	{
		Logger::queueError(Logger::Error,L"Failed to relocate object data");
		return false;
	}

	g_fileManager->advanceMemory(fileData.size());
	return false;
}

void DirectivePspObjImport::Encode()
{
	g_fileManager->write(fileData.data(),fileData.size());
}

void DirectivePspObjImport::writeSymData(SymbolData& symData)
{
	for (size_t i = 0; i < symbols.size(); i++)
	{
		symData.addLabel(symbols[i].address,convertUtf8ToWString(symbols[i].name.c_str()));

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