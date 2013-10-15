#pragma once
#include "Util/FileClasses.h"

class AssemblerFile;

struct SymDataSymbol
{
	std::string name;
	int addressInfoIndex;
	int functionIndex;
};

struct SymDataAddressInfo
{
	int address;
	int fileIndex;
	int lineNumber;
};

struct SymDataFunction
{
	int addressInfoIndex;
	int size;
};

struct SymDataData
{
	int addressInfoIndex;
	int size;
	int type;
};

struct SymDataModule
{
	AssemblerFile* file;
	std::vector<SymDataAddressInfo> addressInfo;
	std::vector<SymDataSymbol> symbols;
	std::vector<SymDataFunction> functions;
	std::vector<SymDataData> data;
};


class SymbolData
{
public:
	enum DataType { Data8, Data16, Data32, DataAscii };

	SymbolData();
	void setNocashSymFileName(const std::wstring& name) { nocashSymFileName = name; };
	void write();
	void setEnabled(bool b) { enabled = b; };

	void addLabel(int address, const std::wstring& name);
	void addData(int address, int size, DataType type);
	void startModule(AssemblerFile* file);
	void endModule(AssemblerFile* file);
	void startFunction(int address);
	void endFunction(int address);
private:
	void writeNocashSym();
	int addAddress(int address);
	int addFileName(const std::string& fileName);

	std::wstring nocashSymFileName;
	bool enabled;

	// entry 0 is for data without parent modules
	std::vector<SymDataModule> modules;
	std::vector<std::string> files;
	int currentModule;
	int currentFunction;
};
