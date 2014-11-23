#include "stdafx.h"
#include "CMacro.h"
#include "Core/SymbolTable.h"
#include "Util/Util.h"

void CMacro::loadArguments(ArgumentList& argumentList)
{
	name = argumentList[0].text;

	for (size_t i = 1; i < argumentList.size(); i++)
	{
		arguments.push_back(argumentList[i].text);
	}
}


std::wstring CMacro::getLine(size_t num, ArgumentList& argumentValues, size_t macroCounter)
{
	std::wstring dest;
	const std::wstring& source = lines[num];
	size_t pos = 0;

	// insert parameters
	while (pos < source.size())
	{
		while (pos < source.size() && !SymbolTable::isValidSymbolCharacter(source[pos],true))
		{
			dest += source[pos++];
		}
		if (pos == source.size()) break;

		std::wstring arg;
		while (pos < source.size() && SymbolTable::isValidSymbolCharacter(source[pos],false))
		{
			arg += source[pos++];
		}

		bool found = false;
		for (size_t i = 0; i < arguments.size(); i++)
		{
			if (arg.compare(arguments[i]) == 0)
			{
				dest += argumentValues[i].text;
				found = true;
				break;
			}
		}

		if (found == false)
			dest += arg;
	}

	// insert labels
	std::wstring dest2;
	pos = 0;
	while (pos < dest.size())
	{
		if (SymbolTable::isLocalSymbol(dest,pos))
		{
			dest += formatString(L"%s_%08X",name,macroCounter);
			pos += 2;
			continue;
		}

		dest2 += dest[pos++];
	}

	return dest2;
}
