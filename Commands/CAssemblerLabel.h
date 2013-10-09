#pragma once
#include "Commands/CAssemblerCommand.h"

class Label;

class CAssemblerLabel: public CAssemblerCommand
{
public:
	CAssemblerLabel(const std::wstring& name, int RamPos, int Section, bool constant);
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
private:
	Label* label;
	bool constant;
};
