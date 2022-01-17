#include "Core/SymbolTable.h"

#include "Core/Assembler.h"
#include "Core/Common.h"
#include "Util/FileClasses.h"
#include "Util/Util.h"

#include <tinyformat.h>

const char validSymbolCharacters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.";

bool operator<(SymbolKey const& lhs, SymbolKey const& rhs)
{
	if (lhs.file != rhs.file)
		return lhs.file < rhs.file;
	if (lhs.section != rhs.section)
		return lhs.section < rhs.section;
	return lhs.name.compare(rhs.name) < 0;
}


Label::Label(const Identifier &name) :
	name(name),
	originalName(name)
{
}


SymbolTable::SymbolTable()
{
	uniqueCount = 0;
}

SymbolTable::~SymbolTable()
{
	clear();
}

void SymbolTable::clear()
{
	symbols.clear();
	labels.clear();
	equationsCount = 0;
	uniqueCount = 0;
}

void SymbolTable::setFileSectionValues(const Identifier& symbol, int& file, int& section)
{
	if (symbol.string()[0] == '@')
	{
		if (symbol.string()[1] != '@')
		{
			// static label, @. the section doesn't matter
			section = -1;
		} else {
			// local label, @@. the file doesn't matter
			file = -1;
		}
	} else {
		// global label. neither file nor section matters
		file = section = -1;
	}
}

std::shared_ptr<Label> SymbolTable::getLabel(const Identifier& symbol, int file, int section)
{
	if (!isValidSymbolName(symbol))
		return nullptr;

	int actualSection = section;
	setFileSectionValues(symbol,file,section);
	SymbolKey key = { symbol, file, section };

	// find label, create new one if it doesn't exist
	auto it = symbols.find(key);
	if (it == symbols.end())
	{
		SymbolInfo value = { LabelSymbol, labels.size() };
		symbols[key] = value;
		
		std::shared_ptr<Label> result = std::make_shared<Label>(Identifier(symbol));
		if (section == actualSection)
			result->setSection(section);			// local, set section of parent
		else
			result->setSection(actualSection+1);	// global, set section of children
		labels.push_back(result);
		return result;
	}

	// make sure not to match symbols that aren't labels
	if (it->second.type != LabelSymbol)
		return nullptr;

	return labels[it->second.index];
}

bool SymbolTable::symbolExists(const Identifier& symbol, int file, int section)
{
	if (!isValidSymbolName(symbol))
		return false;

	setFileSectionValues(symbol,file,section);

	SymbolKey key = { symbol, file, section };
	auto it = symbols.find(key);
	return it != symbols.end();
}

bool SymbolTable::isValidSymbolName(const Identifier& symbol)
{
	const auto& string = symbol.string();

	size_t size = string.size();
	size_t start = 0;

	// don't match empty names
	if (size == 0 || string.compare("@") == 0 || string.compare("@@") == 0)
		return false;

	if (string[0] == '@')
	{
		start++;
		if (size > 1 && string[1] == '@')
			start++;
	}

	if (string[start] >= '0' && string[start] <= '9')
		return false;

	for (size_t i = start; i < size; i++)
	{
		if (strchr(validSymbolCharacters,string[i]) == nullptr)
			return false;
	}

	return true;
}

bool SymbolTable::isValidSymbolCharacter(char character, bool first)
{
	if ((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z')) return true;
	if (!first && (character >= '0' && character <= '9')) return true;
	if (character == '_' || character == '.') return true;
	if (character == '@') return true;
	return false;
}

bool SymbolTable::addEquation(const Identifier& name, int file, int section, size_t referenceIndex)
{
	if (!isValidSymbolName(name))
		return false;

	if (symbolExists(name,file,section))
		return false;
	
	setFileSectionValues(name,file,section);

	SymbolKey key = { name, file, section };
	SymbolInfo value = { EquationSymbol, referenceIndex };
	symbols[key] = value;

	equationsCount++;
	return true;
}

bool SymbolTable::findEquation(const Identifier& name, int file, int section, size_t& dest)
{
	setFileSectionValues(name,file,section);
	
	SymbolKey key = { name, file, section };
	auto it = symbols.find(key);
	if (it == symbols.end() || it->second.type != EquationSymbol)
		return false;

	dest = it->second.index;
	return true;
}

// TODO: better
Identifier SymbolTable::getUniqueLabelName(bool local)
{
	std::string name = tfm::format("__armips_label_%08x__",uniqueCount++);
	if (local)
		name = "@@" + name;

	generatedLabels.insert(Identifier(name));
	return Identifier(name);
}

void SymbolTable::addLabels(const std::vector<LabelDefinition>& labels)
{
	for (const LabelDefinition& def: labels)
	{
		if (!isValidSymbolName(def.name))
			continue;

		std::shared_ptr<Label> label = getLabel(def.name,Global.FileInfo.FileNum,Global.Section);
		if (label == nullptr)
			continue;

		if (!isLocalSymbol(def.name))
			Global.Section++;

		label->setDefined(true);
		label->setValue(def.value);
	}
}

int SymbolTable::findSection(int64_t address)
{
	int64_t smallestBefore = -1;
	int64_t smallestDiff = 0x7FFFFFFF;

	for (auto& lab: labels)
	{
		int64_t diff = address-lab->getValue();
		if (diff >= 0 && diff < smallestDiff)
		{
			smallestDiff = diff;
			smallestBefore = lab->getSection();
		}
	}

	return int(smallestBefore);
}
