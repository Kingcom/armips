#pragma once
#include "Util/CommonClasses.h"

class CMacro
{
public:
	CMacro(void) { counter = 0; };
	~CMacro(void) { };
	void loadArguments(ArgumentList& Arguments);
	void addLine(const std::wstring& str) { lines.push_back(str); };
	std::wstring getLine(int num, ArgumentList& ArgumentValues, int MacroCounter);
	int getLineCount() { return lines.size(); };
	int getArgumentCount() { return arguments.size(); };
	const std::wstring& getName() { return name; };
	int getIncreaseCounter() { return counter++; };
private:
	std::wstring name;
	std::vector<std::wstring> lines;
	std::vector<std::wstring> arguments;
	int counter;
};