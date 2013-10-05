#pragma once
#include "MipsMacros.h"

class CMipsMacro: public CAssemblerCommand
{
public:
	CMipsMacro(int num, tMipsMacroVars& InputVars);
	~CMipsMacro();
	virtual bool Validate();
	virtual void Encode();
	virtual void WriteTempData(FILE*& Output);
private:
	CMipsInstruction* Instructions;
	tMipsMacroData Data;
	int InstructionAmount;
	int SpaceNeeded;
	int MacroNum;
	bool IgnoreLoadDelay;
};