#pragma once
#include "Commands/CAssemblerCommand.h"

class CAssemblerLabel: public CAssemblerCommand
{
public:
	CAssemblerLabel(char* str, int RamPos, int Section, bool constant);
	virtual bool Validate();
	virtual void Encode();
	virtual void WriteTempData(FILE*& Output);
private:
	char LabelName[64];
	int LabelRamPos;
	int LabelSection;
	bool LabelConstant;
};
