#include "stdafx.h"
#include "SymbolData.h"
#include "FileManager.h"
#include "Misc.h"
#include "Common.h"
#include <algorithm>

SymbolData::SymbolData(): enabled(true)
{
	SymDataModule defaultModule;
	defaultModule.file = NULL;
	modules.push_back(defaultModule);
	currentModule = 0;
	currentFunction = -1;
}

struct NocashSymEntry
{
	int address;
	std::string text;

	bool operator<(NocashSymEntry& other)
	{
		if (address != other.address)
			return address < other.address;
		if (other.text[0] == '.')
			return true;
		return false;
	}
};

void SymbolData::writeNocashSym()
{
	if (nocashSymFileName.empty())
		return;

	std::vector<NocashSymEntry> entries;
	for (size_t k = 0; k < modules.size(); k++)
	{
		SymDataModule& module = modules[k];
		for (size_t i = 0; i < module.addressInfo.size(); i++)
		{
			SymDataAddressInfo& info = module.addressInfo[i];
			if (info.symbolIndex != -1)
			{
				NocashSymEntry entry;
				entry.address = info.address;
				entry.text = module.symbols[info.symbolIndex].name;
				entries.push_back(entry);
			}
		}

		for (size_t i = 0; i < module.data.size(); i++)
		{
			SymDataData& data = module.data[i];
			NocashSymEntry entry;
			entry.address = module.addressInfo[data.addressInfoIndex].address;

			switch (data.type)
			{
			case Data8:
				entry.text = formatString(".byt:%04X",data.size);
				break;
			case Data16:
				entry.text = formatString(".wrd:%04X",data.size);
				break;
			case Data32:
				entry.text = formatString(".dbl:%04X",data.size);
				break;
			case DataAscii:
				entry.text = formatString(".asc:%04X",data.size);
				break;
			}

			entries.push_back(entry);
		}
	
		for (size_t i = 0; i < module.specialSymbols.size(); i++)
		{
			SymDataSpecialSymbol& sym = module.specialSymbols[i];
			
			NocashSymEntry entry;
			entry.address = module.addressInfo[sym.addressInfoIndex].address;
			entry.text = sym.name;
			entries.push_back(entry);
		}
	}

	std::sort(entries.begin(),entries.end());
	
	TextFile file;
	if (file.open(nocashSymFileName,TextFile::Write,TextFile::ASCII) == false)
	{
		Logger::printError(Logger::Error,L"Could not open sym file %s.",file.getFileName().c_str());
		return;
	}
	file.writeLine(L"00000000 0");

	for (size_t i = 0; i < entries.size(); i++)
	{
		file.writeFormat(L"%08X %S\n",entries[i].address,entries[i].text.c_str());
	}

	file.write("\x1A");
	file.close();
}

void SymbolData::write()
{
	writeNocashSym();
}

void SymbolData::addLabel(int memoryAddress, const std::wstring& name)
{
	if (!enabled)
		return;

	std::string utf8 = convertWStringToUtf8(name);
	int addressIndex = addAddress(memoryAddress);

	SymDataAddressInfo& info = modules[currentModule].addressInfo[addressIndex];
	if (info.symbolIndex != -1)
	{
		// address already has a symbol. let's just keep that
		return;
	}

	info.symbolIndex = addSymbol(utf8);
}

void SymbolData::addSpecialSymbol(int address, const std::wstring& name)
{
	if (!enabled)
		return;

	SymDataSpecialSymbol sym;
	sym.addressInfoIndex = addAddress(address);
	sym.name = convertWStringToUtf8(name);
	modules[currentModule].specialSymbols.push_back(sym);
}

void SymbolData::addData(int address, int size, DataType type)
{
	if (!enabled)
		return;

	SymDataData data;
	data.addressInfoIndex = addAddress(address);
	data.size = size;
	data.type = type;
	modules[currentModule].data.push_back(data);
}

int SymbolData::addFileName(const std::string& fileName)
{
	for (size_t i = 0; i < files.size(); i++)
	{
		if (files[i] == fileName)
			return i;
	}

	files.push_back(fileName);
	return files.size()-1;
}

int SymbolData::addAddress(int address)
{
	SymDataModule& module = modules[currentModule];

	for (size_t i = 0; i < module.addressInfo.size(); i++)
	{
		if (module.addressInfo[i].address == address)
			return i;
	}

	SymDataAddressInfo info;
	info.address = address;
	info.fileIndex = addFileName(Global.FileInfo.FileList.GetEntry(Global.FileInfo.FileNum));
	info.lineNumber = Global.FileInfo.LineNumber;
	info.symbolIndex = -1;
	module.addressInfo.push_back(info);
	return module.addressInfo.size()-1;
}

int SymbolData::addSymbol(const std::string& name)
{
	SymDataSymbol sym;
	sym.name = name;
	sym.function = currentFunction;
	modules[currentModule].symbols.push_back(sym);
	return modules[currentModule].symbols.size()-1;
}

void SymbolData::startModule(AssemblerFile* file)
{
	for (size_t i = 0; i < modules.size(); i++)
	{
		if (modules[i].file == file)
		{
			currentModule = i;
			return;
		}
	}

	SymDataModule module;
	module.file = file;
	modules.push_back(module);
	currentModule = modules.size()-1;
}

void SymbolData::endModule(AssemblerFile* file)
{
	if (modules[currentModule].file != file)
		return;

	if (currentModule == 0)
	{
		Logger::printError(Logger::Error,L"No module opened");
		return;
	}

	if (currentFunction != -1)
	{
		Logger::printError(Logger::Error,L"Module closed before function end");
		currentFunction = -1;
	}

	currentModule = 0;
}

void SymbolData::startFunction(int address)
{
	if (currentFunction != -1)
	{
		Logger::printError(Logger::Error,L"Opened a function inside another function");
		return;
	}

	currentFunction = modules[currentModule].functions.size();

	SymDataFunction func;
	func.addressInfoIndex = addAddress(address);
	func.size = 0;
	modules[currentModule].functions.push_back(func);
}

void SymbolData::endFunction(int address)
{
	if (currentFunction == -1)
	{
		Logger::printError(Logger::Error,L"Not inside a function");
		return;
	}

	SymDataFunction& func = modules[currentModule].functions[currentFunction];
	func.size = address- modules[currentModule].addressInfo[func.addressInfoIndex].address;
	currentFunction = -1;
}
