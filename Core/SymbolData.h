#pragma once

#include "Util/FileSystem.h"

#include <cstdint>
#include <set>
#include <string>
#include <vector>

class AssemblerFile;

struct SymDataSymbol
{
	std::string name;
	int64_t address;
	
	bool operator<(const SymDataSymbol& other) const
	{
		return address < other.address;
	}
};

struct SymDataAddressInfo
{
	int64_t address;
	size_t fileIndex;
	size_t lineNumber;
	
	bool operator<(const SymDataAddressInfo& other) const
	{
		return address < other.address;
	}
};

struct SymDataFunction
{
	int64_t address;
	size_t size;
	
	bool operator<(const SymDataFunction& other) const
	{
		return address < other.address;
	}
};

struct SymDataData
{
	int64_t address;
	size_t size;
	int type;
	
	bool operator<(const SymDataData& other) const
	{
		if (address != other.address)
			return address < other.address;

		if (size != other.size)
			return size < other.size;

		return type < other.type;
	}
};

struct SymDataModule
{
	AssemblerFile* file;
	std::vector<SymDataSymbol> symbols;
	std::vector<SymDataFunction> functions;
	std::set<SymDataData> data;
};

struct SymDataModuleInfo
{
	unsigned int crc32;
};

class SymbolData
{
public:
	enum DataType { Data8, Data16, Data32, Data64, DataAscii };

	SymbolData();
	void clear();
	void setNocashSymFileName(const fs::path& name, int version) { nocashSymFileName = name; nocashSymVersion = version; };
	void write();
	void setEnabled(bool b) { enabled = b; };

	void addLabel(int64_t address, const std::string& name);
	void addData(int64_t address, size_t size, DataType type);
	void startModule(AssemblerFile* file);
	void endModule(AssemblerFile* file);
	void startFunction(int64_t address);
	void endFunction(int64_t address);
private:
	void writeNocashSym();
	size_t addFileName(const std::string& fileName);

	fs::path nocashSymFileName;
	bool enabled;
	int nocashSymVersion;

	// entry 0 is for data without parent modules
	std::vector<SymDataModule> modules;
	std::vector<std::string> files;
	int currentModule;
	int currentFunction;
};
