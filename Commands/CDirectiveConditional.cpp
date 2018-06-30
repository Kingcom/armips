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
}

CDirectiveConditional::CDirectiveConditional(ConditionType type, const std::wstring& name)
	: CDirectiveConditional(type)
{
	label = Global.symbolTable.getLabel(name,Global.FileInfo.FileNum,Global.Section);
	if (label == nullptr)
		Logger::printError(Logger::Error,L"Invalid label name \"%s\"",name);
}

CDirectiveConditional::CDirectiveConditional(ConditionType type, const Expression& exp)
	: CDirectiveConditional(type)
{
	this->expression = exp;
}

void CDirectiveConditional::setContent(std::unique_ptr<CAssemblerCommand> ifBlock, std::unique_ptr<CAssemblerCommand> elseBlock)
{
	this->ifBlock = std::move(ifBlock);
	this->elseBlock = std::move(elseBlock);
}

bool CDirectiveConditional::evaluate()
{
	int64_t value;
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
	case ConditionType::IF:
		return value != 0;
	case ConditionType::IFDEF:
		return label->isDefined();
	case ConditionType::IFNDEF:
		return !label->isDefined();
	default:
		break;
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
		ifBlock->applyFileInfo();
		if (ifBlock->Validate())
			returnValue = true;
	} else if (elseBlock != nullptr)
	{
		elseBlock->applyFileInfo();
		if (elseBlock->Validate())
			returnValue = true;
	}

	return returnValue;
}

void CDirectiveConditional::Encode() const
{
	if (previousResult)
	{
		ifBlock->Encode();
	} else if (elseBlock != nullptr)
	{
		elseBlock->Encode();
	}
}

void CDirectiveConditional::writeTempData(TempData& tempData) const
{
	if (previousResult)
	{
		ifBlock->applyFileInfo();
		ifBlock->writeTempData(tempData);
	} else if (elseBlock != nullptr)
	{
		elseBlock->applyFileInfo();
		elseBlock->writeTempData(tempData);
	}
}

void CDirectiveConditional::writeSymData(SymbolData& symData) const
{
	if (previousResult)
	{
		ifBlock->writeSymData(symData);
	} else if (elseBlock != nullptr)
	{
		elseBlock->writeSymData(symData);
	}
}
