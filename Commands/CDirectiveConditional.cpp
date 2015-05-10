#include "stdafx.h"
#include "Commands/CDirectiveConditional.h"
#include "Core/Common.h"
#include "Archs/ARM/Arm.h"
#include "Util/Util.h"

extern CArmArchitecture Arm;

CDirectiveConditional::CDirectiveConditional(ConditionType type)
{
	this->type = type;

	ifBlock = nullptr;
	elseBlock = nullptr;
	previousResult = false;

	if (type == ConditionType::IFARM || type == ConditionType::IFTHUMB)
	{
		armState = (Arch == &Arm);
		armState |= (Arm.GetThumbMode() << 1);
	}
}

CDirectiveConditional::CDirectiveConditional(ConditionType type, const std::wstring& name)
	: CDirectiveConditional(type)
{
	label = Global.symbolTable.getLabel(name,Global.FileInfo.FileNum,Global.Section);
	if (label == NULL)
		Logger::printError(Logger::Error,L"Invalid label name \"%s\"",name);
}

CDirectiveConditional::CDirectiveConditional(ConditionType type, const Expression& exp)
	: CDirectiveConditional(type)
{
	this->expression = exp;
}

CDirectiveConditional::~CDirectiveConditional()
{
	delete ifBlock;
	delete elseBlock;
}

void CDirectiveConditional::setContent(CAssemblerCommand* ifBlock, CAssemblerCommand* elseBlock)
{
	this->ifBlock = ifBlock;
	this->elseBlock = elseBlock;
}

bool CDirectiveConditional::evaluate()
{
	u64 value;
	if (expression.isLoaded())
	{
		if (expression.evaluateInteger(value) == false)
		{
			Logger::queueError(Logger::Error,L"Invalid conditional expression");
			return false;
		}
	}

	switch (type)
	{
	case ConditionType::IFARM:
		return armState == 1;
	case ConditionType::IFTHUMB:
		return armState == 3;
	case ConditionType::IF:
		return value != 0;
	case ConditionType::IFDEF:
		return label->isDefined();
	case ConditionType::IFNDEF:
		return !label->isDefined();
	}
			
	Logger::queueError(Logger::Error,L"Invalid conditional type");
	return false;
}

bool CDirectiveConditional::Validate()
{
	bool result = evaluate();
	bool returnValue = result != previousResult;
	previousResult = result;

	if (result)
	{
		if (ifBlock->Validate())
			returnValue = true;
	} else if (elseBlock != NULL)
	{
		if (elseBlock->Validate())
			returnValue = true;
	}

	return returnValue;
}

void CDirectiveConditional::Encode() const
{
	if (previousResult)
		ifBlock->Encode();
	else if (elseBlock != NULL)
		elseBlock->Encode();
}
