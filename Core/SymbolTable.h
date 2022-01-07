#pragma once

#include "Core/Types.h"

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

struct LabelDefinition;

struct SymbolKey
{
	SymbolKey(const Identifier& name, int file, int section) :
		name(name.string()),
		file(file),
		section(section)
	{
		std::transform(this->name.begin(), this->name.end(), this->name.begin(), ::tolower);
	}

	std::string name;
	int file;
	int section;
};

bool operator<(SymbolKey const& lhs, SymbolKey const& rhs);

class Label
{
public:
	Label(const Identifier&	name);
	const Identifier &getName() { return name; };
	void setOriginalName(const Identifier& name) { originalName = name; }
	const Identifier &getOriginalName() { return originalName; }
	int64_t getValue() { return value; };
	void setValue(int64_t val) { value = val; };
	bool hasPhysicalValue() { return physicalValueSet; }
	int64_t getPhysicalValue() { return physicalValue; }
	void setPhysicalValue(int64_t val) { physicalValue = val; physicalValueSet = true; }
	bool isDefined() { return defined; };
	void setDefined(bool b) { defined = b; };
	bool isData() { return data; };
	void setIsData(bool b) { data = b; };
	void setInfo(int inf) { info = inf; };
	int getInfo() { return info; };
	void setUpdateInfo(bool b) { updateInfo = b; };
	bool getUpdateInfo() { return updateInfo; };
	void setSection(int num) { section = num; }
	int getSection() { return section; }
private:
	Identifier name;
	Identifier originalName;
	int64_t value = 0;
	int64_t physicalValue = 0;
	bool physicalValueSet = false;
	bool defined = false;
	bool data = false;
	bool updateInfo = true;
	int info = 0;
	int section = 0;
};

class SymbolTable
{
public:
	SymbolTable();
	~SymbolTable();
	void clear();
	bool symbolExists(const Identifier& symbol, int file, int section);
	static bool isValidSymbolName(const Identifier& symbol);
	static bool isValidSymbolCharacter(char character, bool first = false);
	static bool isLocalSymbol(const Identifier& symbol) { return symbol.size() >= 2 && symbol.string()[0] == '@' && symbol.string()[1] == '@'; }
	static bool isStaticSymbol(const Identifier& symbol) { return symbol.size() >= 1 && symbol.string()[0] == '@'; }
	static bool isGlobalSymbol(const Identifier& symbol) { return !isLocalSymbol(symbol) && !isStaticSymbol(symbol); }

	std::shared_ptr<Label> getLabel(const Identifier& symbol, int file, int section);
	bool addEquation(const Identifier& name, int file, int section, size_t referenceIndex);
	bool findEquation(const Identifier& name, int file, int section, size_t& dest);
	void addLabels(const std::vector<LabelDefinition>& labels);
	int findSection(int64_t address);

	Identifier getUniqueLabelName(bool local = false);
	size_t getLabelCount() { return labels.size(); };
	size_t getEquationCount() { return equationsCount; };
	bool isGeneratedLabel(const Identifier& name) { return generatedLabels.find(name) != generatedLabels.end(); }
private:
	void setFileSectionValues(const Identifier& symbol, int& file, int& section);

	enum SymbolType { LabelSymbol, EquationSymbol };
	struct SymbolInfo
	{
		SymbolType type;
		size_t index;
	};

	std::map<SymbolKey,SymbolInfo> symbols;
	std::vector<std::shared_ptr<Label>> labels;
	size_t equationsCount;
	size_t uniqueCount;
	std::set<Identifier> generatedLabels;
};
