#pragma once
#include "Util/CommonClasses.h"

class CMacro
{
public:
	CMacro(void) { counter = 0; };
	~CMacro(void) { };
	void loadArguments(ArgumentList& Arguments);
	void addLine(const std::wstring& str) { lines.push_back(str); };
	std::wstring getLine(size_t num, ArgumentList& ArgumentValues, size_t MacroCounter);
	size_t getLineCount() { return lines.size(); };
	size_t getArgumentCount() { return arguments.size(); };
	const std::wstring& getName() { return name; };
	size_t getIncreaseCounter() { return counter++; };
private:
	std::wstring name;
	std::vector<std::wstring> lines;
	std::vector<std::wstring> arguments;
	size_t counter;
};