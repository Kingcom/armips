#pragma once
#include "Commands/CAssemblerCommand.h"

class Label;

class CAssemblerLabel: public CAssemblerCommand
{
public:
	CAssemblerLabel(const std::wstring& name, int RamPos, int Section, bool constant);
	virtual bool Validate();
	virtual void Encode();
	virtual void WriteTempData(FILE*& Output);
private:
	Label* label;
	bool constant;
};
