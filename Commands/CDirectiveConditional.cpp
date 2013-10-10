#include "stdafx.h"
#include "Commands/CDirectiveConditional.h"
#include "Core/Common.h"
#include "Core/MathParser.h"
#include "Archs/ARM/Arm.h"
#include "Util/Util.h"

extern CArmArchitecture Arm;

CDirectiveConditional::CDirectiveConditional()
{
	Value = 0;
	IsNum = true;
	IsNumSolved = false;
}

bool CDirectiveConditional::Load(ArgumentList& Args, int command)
{
	CStringList List;

	Type = command;

	switch (Type)
	{
	case CONDITIONAL_IF:
	case CONDITIONAL_ELSEIF:
		if (initExpression(Expression,Args[0].text) == false)
			return false;
		break;
	case CONDITIONAL_IFDEF:
	case CONDITIONAL_IFNDEF:
	case CONDITIONAL_ELSEIFDEF:
	case CONDITIONAL_ELSEIFNDEF:
		if (Global.symbolTable.isValidSymbolName(Args[0].text) == false)
		{
			Logger::printError(Logger::Error,L"Invalid label name \"%s\"",Args[0].text.c_str());
			return false;
		}
		labelName = Args[0].text;
		break;
	case CONDITIONAL_IFARM:
	case CONDITIONAL_IFTHUMB:
		Value = (Arch == &Arm);
		Value |= (Arm.GetThumbMode() << 1);
		break;
	}
	return true;
}

void CDirectiveConditional::Execute()
{
	bool b;

	switch (Type)
	{
	case CONDITIONAL_IFARM:
		b = Value == 1;
		Global.conditionData.addIf(b);
		break;
	case CONDITIONAL_IFTHUMB:
		b = Value == 3;
		Global.conditionData.addIf(b);
		break;
	case CONDITIONAL_IF:
		b = Value != 0;
		Global.conditionData.addIf(b);
		break;
	case CONDITIONAL_ELSE:
		Global.conditionData.addElse();
		break;
	case CONDITIONAL_ELSEIF:
		b = Value != 0;
		Global.conditionData.addElseIf(b);
		break;
	case CONDITIONAL_ENDIF:
		Global.conditionData.addEndIf();
		break;
	case CONDITIONAL_IFDEF:
		b = checkLabelDefined(labelName);
		Global.conditionData.addIf(b);
		break;
	case CONDITIONAL_IFNDEF:
		b = !checkLabelDefined(labelName);
		Global.conditionData.addIf(b);
		break;
	case CONDITIONAL_ELSEIFDEF:	
		b = checkLabelDefined(labelName);
		Global.conditionData.addElseIf(b);
		break;
	case CONDITIONAL_ELSEIFNDEF:
		b = !checkLabelDefined(labelName);
		Global.conditionData.addElseIf(b);
		break;
	}
}

bool CDirectiveConditional::Validate()
{
	bool Result = false;
	int num;

	switch (Type)
	{
	case CONDITIONAL_IF:
	case CONDITIONAL_ELSEIF:
		if (ParsePostfix(Expression,NULL,num) == false)
		{
			Logger::printError(Logger::Error,L"Invalid expression");
			return false;
		}

		if (Value != num) Result = true;
		Value = num;
		break;
	}

	Execute();
	return Result;
}

void CDirectiveConditional::Encode()
{
	Execute();
}
