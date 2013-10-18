#pragma once
#include "MipsMacros.h"

class CMipsMacro: public CAssemblerCommand
{
public:
	CMipsMacro(int num, tMipsMacroVars& InputVars);
	~CMipsMacro();
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
private:
	CMipsInstruction* Instructions;
	tMipsMacroData Data;
	int InstructionAmount;
	int MacroNum;
	bool IgnoreLoadDelay;
};