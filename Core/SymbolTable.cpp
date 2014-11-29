#include "stdafx.h"
#include "Core/SymbolTable.h"
#include "Util/FileClasses.h"
#include "Util/Util.h"
#include "Common.h"

const wchar_t validSymbolCharacters[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.";

bool operator<(SymbolKey const& lhs, SymbolKey const& rhs)
{
	if (lhs.file != rhs.file)
		return lhs.file < rhs.file;
	if (lhs.section != rhs.section)
		return lhs.section < rhs.section;
	return lhs.name.compare(rhs.name) < 0;
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
	for (size_t i = 0; i < labels.size(); i++)
	{
		delete labels[i];
	}

	symbols.clear();
	labels.clear();
	equations.clear();
	uniqueCount = 0;
}

void SymbolTable::setFileSectionValues(const std::wstring& symbol, unsigned int& file, unsigned int& section)
{
	if (symbol[0] == '@')
	{
		if (symbol[1] != '@')
		{
			// static label, @. the section doesn't matter
			section = -1;
		}
	} else {
		// global label. neither file nor section matters
		file = section = -1;
	}
}

Label* SymbolTable::getLabel(const std::wstring& symbol, unsigned int file, unsigned int section)
{
	if (isValidSymbolName(symbol) == false)
		return NULL;

	int actualSection = section;
	setFileSectionValues(symbol,file,section);
	SymbolKey key = { symbol, file, section };

	// find label, create new one if it doesn't exist
	auto it = symbols.find(key);
	if (it == symbols.end())
	{
		SymbolInfo value = { LabelSymbol, labels.size() };
		symbols[key] = value;
		
		Label* result = new Label(symbol);
		if (section == actualSection)
			result->setSection(section);			// local, set section of parent
		else
			result->setSection(actualSection+1);	// global, set section of children
		labels.push_back(result);
		return result;
	}

	// make sure not to match symbols that aren't labels
	if (it->second.type != LabelSymbol)
		return NULL;

	return labels[it->second.index];
}

bool SymbolTable::symbolExists(const std::wstring& symbol, unsigned int file, unsigned int section)
{
	if (isValidSymbolName(symbol) == false)
		return false;

	setFileSectionValues(symbol,file,section);

	SymbolKey key = { symbol, file, section };
	auto it = symbols.find(key);
	return it != symbols.end();
}

bool SymbolTable::isValidSymbolName(const std::wstring& symbol)
{
	size_t size = symbol.size();
	size_t start = 0;

	// don't match empty names
	if (size == 0 || symbol.compare(L"@") == 0 || symbol.compare(L"@@") == 0)
		return false;

	if (symbol[0] == '@')
	{
		start++;
		if (size > 1 && symbol[1] == '@')
			start++;
	}

	if (symbol[start] >= '0' && symbol[start] <= '9')
		return false;

	for (size_t i = start; i < size; i++)
	{
		if (wcschr(validSymbolCharacters,symbol[i]) == NULL)
			return false;
	}

	return true;
}

bool SymbolTable::isValidSymbolCharacter(wchar_t character, bool first)
{
	character = towlower(character);
	if (character >= 'a' && character <= 'z') return true;
	if (!first && character >= '0' && character <= '9') return true;
	if (character == '_' || character == '.') return true;
	return false;
}

bool SymbolTable::addEquation(const std::wstring& name, unsigned int file, unsigned int section, std::wstring& replacement)
{
	if (isValidSymbolName(name) == false)
		return false;

	if (symbolExists(name,file,section))
		return false;
	
	setFileSectionValues(name,file,section);

	SymbolKey key = { name, file, section };
	SymbolInfo value = { EquationSymbol, equations.size() };
	symbols[key] = value;

	Equation equation = { name, replacement, file, section };
	equations.push_back(equation);
	return true;
}

std::wstring SymbolTable::insertEquations(const std::wstring& line, unsigned int file, unsigned int section)
{
	std::wstring result;

	size_t pos = 0;
	while (pos < line.size())
	{
		if (line[pos] != '@' && !isValidSymbolCharacter(line[pos]))
		{
			result += line[pos++];
			continue;
		}

		size_t start = pos++;
		while (line[pos] == '@' && pos < line.size())
			pos++;
		while (isValidSymbolCharacter(line[pos]) && pos < line.size())
			pos++;

		std::wstring word = line.substr(start,pos-start);
		bool found = false;
		for (size_t i = 0; i < equations.size(); i++)
		{
			const Equation& eq = equations.at(i);
			if ((eq.file == -1 || eq.file == file) &&
				(eq.section == -1 || eq.section == section))
			{
				if (eq.key.size() != word.size())
					continue;

				if (eq.key != word)
					continue;
				
				result += eq.value;
				found = true;
				break;
			}
		}

		if (!found)
			result += word;
	}

	return result;
}

// TODO: better
std::wstring SymbolTable::getUniqueLabelName()
{
	return formatString(L"__armips_label_%08X__",uniqueCount++);
}

void SymbolTable::addLabels(const std::vector<LabelDefinition>& labels)
{
	int lastSection = 0;
	for (const LabelDefinition& def: labels)
	{
		if (!isValidSymbolName(def.name))
			continue;

		Label* label = getLabel(def.name,(unsigned int)Global.FileInfo.FileNum,Global.Section);
		if (label == NULL)
			continue;

		if (isLocalSymbol(def.name) == false)
			Global.Section++;

		label->setDefined(true);
		label->setValue(def.value);
	}
}

int SymbolTable::findSection(u64 address)
{
	int smallestBefore = -1;
	int smallestDiff = 0x7FFFFFFF;

	for (auto& lab: labels)
	{
		int diff = (int)(address-lab->getValue());
		if (diff >= 0 && diff < smallestDiff)
		{
			smallestDiff = diff;
			smallestBefore = lab->getSection();
		}
	}

	return smallestBefore;
}