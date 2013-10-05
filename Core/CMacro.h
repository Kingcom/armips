#pragma once
#include "Util/CommonClasses.h"

class CMacro
{
public:
	CMacro(void) { Counter = 0; };
	~CMacro(void) { };
	void LoadArguments(CArgumentList& Arguments);
	void AddLine(char* str) { Lines.AddEntry(str); };
	void GetLine(int num, CArgumentList& ArgumentValues, char* dest, int MacroCounter);
	int GetLineCount() { return Lines.GetCount(); };
	int GetArgumentCount() { return Arguments.GetCount(); };
	char* GetName() { return Name; };
	int GetIncreaseCounter() { return Counter++; };
private:
	char Name[64];
	CStringList Lines;
	CStringList Arguments;
	int Counter;
};