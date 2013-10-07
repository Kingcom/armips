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
	Label(std::wstring name): name(name), defined(false) { };
	const std::wstring getName() { return name; };
	unsigned int getValue() { return value; };
	void setValue(unsigned int val) { value = val; };
	bool isDefined() { return defined; };
	void setDefined(bool b) { defined = b; };
private:
	std::wstring name;
	unsigned int value;
	bool defined;
};

class SymbolTable
{
public:
	SymbolTable();
	~SymbolTable();
	bool symbolExists(const std::wstring& symbol, int file, int section);
	bool isValidSymbolName(const std::wstring& symbol);
	bool isLocalSymbol(const std::wstring& symbol) { return symbol.size() >= 2 && symbol[0] == '@' && symbol[1] == '@'; };
	bool isStaticSymbol(const std::wstring& symbol) { return symbol.size() >= 1 && symbol[0] == '@'; };

	Label* getLabel(const std::wstring& symbol, int file, int section);
	bool addEquation(const std::wstring& name, int file, int section, std::wstring& replacement);
	std::wstring insertEquations(const std::wstring& line, int file, int section);

	void writeSymFile(const std::string fileName);
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
};
