#pragma once
#include "../Util/FileClasses.h"

class AssemblerFile;

struct SymDataSymbol
{
	std::wstring name;
	u64 address;
	
	bool operator<(const SymDataSymbol& other) const
	{
		return address < other.address;
	}
};

struct SymDataAddressInfo
{
	u64 address;
	size_t fileIndex;
	size_t lineNumber;
	
	bool operator<(const SymDataAddressInfo& other) const
	{
		return address < other.address;
	}
};

struct SymDataFunction
{
	u64 address;
	size_t size;
	
	bool operator<(const SymDataFunction& other) const
	{
		return address < other.address;
	}
};

struct SymDataData
{
	u64 address;
	size_t size;
	int type;
	
	bool operator<(const SymDataData& other) const
	{
		return address < other.address;
	}
};

struct SymDataModule
{
	AssemblerFile* file;
	std::vector<SymDataAddressInfo> addressInfo;
	std::vector<SymDataSymbol> symbols;
	std::vector<SymDataFunction> functions;
	std::vector<SymDataData> data;
};

struct SymDataModuleInfo
{
	unsigned int crc32;
};

class SymbolData
{
public:
	enum DataType { Data8, Data16, Data32, DataAscii };

	SymbolData();
	void clear();
	void setNocashSymFileName(const std::wstring& name, int version) { nocashSymFileName = name; nocashSymVersion = version; };
	void write();
	void setEnabled(bool b) { enabled = b; };

	void addLabel(u64 address, const std::wstring& name);
	void addData(u64 address, size_t size, DataType type);
	void startModule(AssemblerFile* file);
	void endModule(AssemblerFile* file);
	void startFunction(u64 address);
	void endFunction(u64 address);
private:
	void writeNocashSym();
	size_t addAddress(u64 address);
	size_t addFileName(const std::string& fileName);

	std::wstring nocashSymFileName;
	bool enabled;
	int nocashSymVersion;

	// entry 0 is for data without parent modules
	std::vector<SymDataModule> modules;
	std::vector<std::string> files;
	size_t currentModule;
	size_t currentFunction;
};
