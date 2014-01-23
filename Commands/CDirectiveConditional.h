#pragma once
#include "Util/CommonClasses.h"
#include "Commands/CAssemblerCommand.h"
#include "Core/MathParser.h"

enum class ConditionType
{
	IF,
	ELSE,
	ELSEIF,
	ENDIF,
	IFDEF,
	IFNDEF,
	ELSEIFDEF,
	ELSEIFNDEF,
	IFARM,
	IFTHUMB
};

class CDirectiveConditional: public CAssemblerCommand
{
public:
	CDirectiveConditional();
	bool Load(ArgumentList& Args, ConditionType command);
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData) { };
	virtual bool IsConditional() { return true; };
private:
	void Execute();
	CExpressionCommandList Expression;
	std::wstring labelName;
	int Value;
	int RamPos;
	ConditionType type;
};
