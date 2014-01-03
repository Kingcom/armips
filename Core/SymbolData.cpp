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
		for (size_t i = 0; i < module.symbols.size(); i++)
		{
			SymDataSymbol& sym = module.symbols[i];
			
			int size = 0;
			for (size_t f = 0; f < module.functions.size(); f++)
			{
				if (module.functions[f].address == sym.address)
				{
					size = module.functions[f].size;
					break;
				}
			}

			NocashSymEntry entry;
			entry.address = sym.address;

			if (size != 0 && nocashSymVersion >= 2)
				entry.text = formatString("%s,%08X",sym.name.c_str(),size);
			else
				entry.text = sym.name;

			entries.push_back(entry);
		}

		for (size_t i = 0; i < module.data.size(); i++)
		{
			SymDataData& data = module.data[i];
			NocashSymEntry entry;
			entry.address = data.address;

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
	writeExSym();
}

void SymbolData::addLabel(int memoryAddress, const std::wstring& name)
{
	if (!enabled)
		return;
	addAddress(memoryAddress);

	SymDataSymbol sym;
	sym.address = memoryAddress;
	sym.name = convertWStringToUtf8(name);
	modules[currentModule].symbols.push_back(sym);
}

void SymbolData::addData(int address, int size, DataType type)
{
	if (!enabled)
		return;
	addAddress(address);

	SymDataData data;
	data.address = address;
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
	module.addressInfo.push_back(info);
	return module.addressInfo.size()-1;
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
		endFunction(address);
	}

	currentFunction = modules[currentModule].functions.size();
	addAddress(address);

	SymDataFunction func;
	func.address = address;
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
	func.size = address-func.address;
	currentFunction = -1;
}






typedef unsigned int u32;
typedef unsigned char u8;
typedef unsigned short u16;

struct ExSymHeader
{
	u8 magic[4];
	u16 version;
	u16 headerSize;	// or whatever
	u32 modulePos;
	u32 moduleCount;
	u32 filesPos;
	u32 fileCount;
	u32 stringTablePos;
};

struct ExSymModuleHeader
{
	u32 crc32;
	u32 addressInfosPos;
	u32 addressInfosCount;
	u32 functionsPos;
	u32 functionsCount;
	u32 symbolsPos;
	u32 symbolsCount;
	u32 datasPos;
	u32 datasCount;
};

struct ExSymAddressInfoEntry
{
	u32 address;
	u32 fileIndex;
	u32 lineNumber;
};

struct ExSymFunctionEntry
{
	u32 addressInfoIndex;
	u32 size;
};

struct ExSymDataEntry
{
	u32 addressInfoIndex;
	u32 size:28;
	u32 type:4;
};

struct ExSymSymbolEntry
{
	u32 addressInfoIndex;
	u32 functionIndex;	// -1 if none
	u32 namePos:28;
	u32 type:4;
};

struct ExSymFileEntry
{
	u32 filePos;
};


int getAddressInfoEntry(std::vector<SymDataAddressInfo>& addresses, int address)
{
	for (size_t i = 0; i < addresses.size(); i++)
	{
		if (addresses[i].address == address)
			return i;
	}

	return -1;
}

int getFunctionEntry(std::vector<SymDataFunction>& functions, int address)
{
	for (size_t i = 0; i < functions.size(); i++)
	{
		if (functions[i].address <= address &&
			(functions[i].address+functions[i].size) > address)
			return i;
	}

	return -1;
}

void SymbolData::writeExSym()
{
	if (exSymFileName.empty())
		return;

	ByteArray data;
	ByteArray stringData;

	data.reserveBytes(sizeof(ExSymHeader));

	ExSymHeader header;
	memcpy(header.magic,"ESYM",4);
	header.version = 1;
	header.headerSize = sizeof(ExSymHeader);
	header.modulePos = data.size();
	header.moduleCount = modules.size();
	data.reserveBytes(header.moduleCount*sizeof(ExSymModuleHeader));

	// add modules
	for (size_t i = 0; i < modules.size(); i++)
	{
		SymDataModule& module = modules[i];

		ExSymModuleHeader moduleHeader;
		int modulePos = header.modulePos+i*sizeof(ExSymModuleHeader);

		std::sort(module.addressInfo.begin(),module.addressInfo.end());
		std::sort(module.data.begin(),module.data.end());
		std::sort(module.functions.begin(),module.functions.end());
		std::sort(module.symbols.begin(),module.symbols.end());

		SymDataModuleInfo moduleInfo;
		if (module.file != NULL && module.file->getModuleInfo(moduleInfo))
		{
			moduleHeader.crc32 = moduleInfo.crc32;
		} else {
			moduleHeader.crc32 = -1;
		}

		// add address info
		moduleHeader.addressInfosPos = data.size();
		moduleHeader.addressInfosCount = module.addressInfo.size();
		for (size_t l = 0; l < module.addressInfo.size(); l++)
		{
			SymDataAddressInfo& info = module.addressInfo[l];

			ExSymAddressInfoEntry entry;
			entry.address = info.address;
			entry.fileIndex = info.fileIndex;
			entry.lineNumber = info.lineNumber;
			data.append((byte*)&entry,sizeof(entry));
		}

		// add symbols
		moduleHeader.symbolsPos = data.size();
		moduleHeader.symbolsCount = module.symbols.size();
		for (size_t l = 0; l < module.symbols.size(); l++)
		{
			SymDataSymbol& sym = module.symbols[l];

			ExSymSymbolEntry entry;
			entry.addressInfoIndex = getAddressInfoEntry(module.addressInfo,sym.address);
			entry.functionIndex = getFunctionEntry(module.functions,sym.address);
			entry.namePos = stringData.size();
			stringData.append((byte*)sym.name.c_str(),sym.name.size()+1);
			entry.type = 0;
			data.append((byte*)&entry,sizeof(entry));
		}

		// add functions
		moduleHeader.functionsPos = data.size();
		moduleHeader.functionsCount = module.functions.size();
		for (size_t l = 0; l < module.functions.size(); l++)
		{
			SymDataFunction& func = module.functions[l];

			ExSymFunctionEntry entry;
			entry.addressInfoIndex = getAddressInfoEntry(module.addressInfo,func.address);
			entry.size = func.size;
			data.append((byte*)&entry,sizeof(entry));
		}

		// add data
		moduleHeader.datasPos = data.size();
		moduleHeader.datasCount = module.data.size();
		for (size_t l = 0; l < module.data.size(); l++)
		{
			SymDataData& d = module.data[l];

			ExSymDataEntry entry;
			entry.addressInfoIndex = getAddressInfoEntry(module.addressInfo,d.address);
			entry.size = d.size;
			entry.type = d.type;
			data.append((byte*)&entry,sizeof(entry));
		}

		// write header
		data.replaceBytes(modulePos,(byte*)&moduleHeader,sizeof(moduleHeader));
	}

	// add files
	header.filesPos = data.size();
	header.fileCount = files.size();
	for (size_t i = 0; i < files.size(); i++)
	{
		ExSymFileEntry entry;
		entry.filePos = stringData.size();
		stringData.append((byte*)files[i].c_str(),files[i].size()+1);
		data.append((byte*)&entry,sizeof(entry));
	}

	// add string table
	header.stringTablePos = data.size();
	data.append(stringData);
	
	// write header
	data.replaceBytes(0,(byte*)&header,sizeof(header));

	// write
	data.toFile(exSymFileName);
}

