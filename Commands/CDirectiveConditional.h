#pragma once
#include "Util/CommonClasses.h"
#include "Commands/CAssemblerCommand.h"
#include "Core/MathParser.h"

#define CONDITIONAL_IF				0x00000001
#define CONDITIONAL_ELSE			0x00000002
#define CONDITIONAL_ELSEIF			0x00000003
#define CONDITIONAL_ENDIF			0x00000004
#define CONDITIONAL_IFDEF			0x00000005
#define CONDITIONAL_IFNDEF			0x00000006
#define CONDITIONAL_ELSEIFDEF		0x00000007
#define CONDITIONAL_ELSEIFNDEF		0x00000008
#define CONDITIONAL_IFARM			0x00000009
#define CONDITIONAL_IFTHUMB			0x0000000A

class CDirectiveConditional: public CAssemblerCommand
{
public:
	CDirectiveConditional();
	bool Load(ArgumentList& Args, int command);
	virtual bool Validate();
	virtual void Encode();
	virtual void WriteTempData(FILE*& Output);
	virtual bool IsConditional() { return true; };
private:
	void Execute();
	CExpressionCommandList Expression;
	std::wstring labelName;
	int Value;
	int Type;
	int RamPos;
	bool IsNum;
	bool IsNumSolved;
};
