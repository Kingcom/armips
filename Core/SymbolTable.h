#pragma once

#include <map>

struct SymbolKey
{
	std::wstring name;
	unsigned int file;
	unsigned int section;
};

bool operator<(SymbolKey const& lhs, SymbolKey const& rhs);

class Label
{
public:
	Label(std::wstring name): name(name), defined(false),info(0),data(false),updateInfo(true) { };
	const std::wstring getName() { return name; };
	unsigned int getValue() { return value; };
	void setValue(unsigned int val) { value = val; };
	bool isDefined() { return defined; };
	void setDefined(bool b) { defined = b; };
	bool isData() { return data; };
	void setIsData(bool b) { data = b; };
	void setInfo(int inf) { info = inf; };
	int getInfo() { return info; };
	void setUpdateInfo(bool b) { updateInfo = b; };
	bool getUpdateInfo() { return updateInfo; };
private:
	std::wstring name;
	unsigned int value;
	bool defined;
	bool data;
	int info;
	bool updateInfo;
};

class SymbolTable
{
public:
	SymbolTable();
	~SymbolTable();
	void clear();
	bool symbolExists(const std::wstring& symbol, int file, int section);
	static bool isValidSymbolName(const std::wstring& symbol);
	static bool isValidSymbolCharacter(wchar_t character, bool first = false);
	static bool isLocalSymbol(const std::wstring& symbol, size_t pos = 0) { return symbol.size() >= pos+2 && symbol[pos+0] == '@' && symbol[pos+1] == '@'; };
	static bool isStaticSymbol(const std::wstring& symbol, size_t pos = 0) { return symbol.size() >= pos+1 && symbol[pos+0] == '@'; };
	static bool isGlobalSymbol(const std::wstring& symbol, size_t pos = 0) { return !isLocalSymbol(symbol) && !isStaticSymbol(symbol); };

	Label* getLabel(const std::wstring& symbol, int file, int section);
	bool addEquation(const std::wstring& name, int file, int section, std::wstring& replacement);
	std::wstring insertEquations(const std::wstring& line, int file, int section);

	std::wstring getUniqueLabelName();
	void writeSymFile(const std::string& fileName);
	size_t getLabelCount() { return labels.size(); };
	size_t getEquationCount() { return equations.size(); };
private:
	void setFileSectionValues(const std::wstring& symbol, int& file, int& section);

	enum SymbolType { LabelSymbol, EquationSymbol };
	struct SymbolInfo
	{
		SymbolType type;
		int index;
	};

	struct Equation
	{
		std::wstring key;
		std::wstring value;
		int file;
		int section;
	};

	std::map<SymbolKey,SymbolInfo> symbols;
	std::vector<Label*> labels;
	std::vector<Equation> equations;
	int uniqueCount;
};
