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
}

bool CDirectiveConditional::Load(ArgumentList& Args, ConditionType command)
{
	type = command;

	switch (type)
	{
	case ConditionType::IF:
	case ConditionType::ELSEIF:
		if (initExpression(Expression,Args[0].text) == false)
			return false;
		break;
	case ConditionType::IFDEF:
	case ConditionType::IFNDEF:
	case ConditionType::ELSEIFDEF:
	case ConditionType::ELSEIFNDEF:
		if (Global.symbolTable.isValidSymbolName(Args[0].text) == false)
		{
			Logger::printError(Logger::Error,L"Invalid label name \"%s\"",Args[0].text);
			return false;
		}
		labelName = Args[0].text;
		break;
	case ConditionType::IFARM:
	case ConditionType::IFTHUMB:
		Value = (Arch == &Arm);
		Value |= (Arm.GetThumbMode() << 1);
		break;
	}
	return true;
}

void CDirectiveConditional::Execute()
{
	bool b;

	switch (type)
	{
	case ConditionType::IFARM:
		b = Value == 1;
		Global.conditionData.addIf(b);
		break;
	case ConditionType::IFTHUMB:
		b = Value == 3;
		Global.conditionData.addIf(b);
		break;
	case ConditionType::IF:
		b = Value != 0;
		Global.conditionData.addIf(b);
		break;
	case ConditionType::ELSE:
		Global.conditionData.addElse();
		break;
	case ConditionType::ELSEIF:
		b = Value != 0;
		Global.conditionData.addElseIf(b);
		break;
	case ConditionType::ENDIF:
		Global.conditionData.addEndIf();
		break;
	case ConditionType::IFDEF:
		b = checkLabelDefined(labelName);
		Global.conditionData.addIf(b);
		break;
	case ConditionType::IFNDEF:
		b = !checkLabelDefined(labelName);
		Global.conditionData.addIf(b);
		break;
	case ConditionType::ELSEIFDEF:	
		b = checkLabelDefined(labelName);
		Global.conditionData.addElseIf(b);
		break;
	case ConditionType::ELSEIFNDEF:
		b = !checkLabelDefined(labelName);
		Global.conditionData.addElseIf(b);
		break;
	}
}

bool CDirectiveConditional::Validate()
{
	bool Result = false;
	int num;

	switch (type)
	{
	case ConditionType::IF:
	case ConditionType::ELSEIF:
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
