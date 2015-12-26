#include "stdafx.h"
#include "Commands/CAssemblerLabel.h"
#include "Core/Common.h"
#include "Util/Util.h"
#include "Core/FileManager.h"
#include "Archs/ARM/Arm.h"

CAssemblerLabel::CAssemblerLabel(const std::wstring& name)
{
	this->defined = false;
	this->label = NULL;
	
	if (Global.symbolTable.isLocalSymbol(name) == false)	
		updateSection(++Global.Section);

	label = Global.symbolTable.getLabel(name, FileNum, getSection());
	if (label == NULL)
	{
		Logger::printError(Logger::Error, L"Invalid label name \"%s\"", name);
		return;
	}

	// does this need to be in validate?
	if (label->getUpdateInfo())
	{
		if (Arch == &Arm && Arm.GetThumbMode())
			label->setInfo(1);
		else
			label->setInfo(0);
	}
}

CAssemblerLabel::CAssemblerLabel(const std::wstring& name, Expression& value)
	: CAssemblerLabel(name)
{
	labelValue = value;
}

bool CAssemblerLabel::Validate()
{
	bool result = false;
	if (defined == false)
	{
		if (label->isDefined())
		{
			Logger::queueError(Logger::Error, L"Label \"%s\" already defined", label->getName());
			return false;
		}
		
		label->setDefined(true);
		defined = true;
		result = true;
	}
	
	u64 value;
	if (labelValue.isLoaded())
	{
		// label value is given by expression
		if (labelValue.evaluateInteger(value) == false)
		{
			Logger::printError(Logger::Error, L"Invalid expression");
			return result;
		}
	} else {
		// label value is given by current address
		value = g_fileManager->getVirtualAddress();
	}

	if (label->getValue() != value)
	{
		label->setValue(value);
		result = true;
	}

	return result;
}

void CAssemblerLabel::Encode() const
{

}

void CAssemblerLabel::writeTempData(TempData& tempData) const
{
	tempData.writeLine(label->getValue(),formatString(L"%s:",label->getName()));
}

void CAssemblerLabel::writeSymData(SymbolData& symData) const
{
	// TODO: find a less ugly way to check for undefined memory positions
	if (label->getValue() == (u64)-1)
		return;

	symData.addLabel(label->getValue(),label->getName());
}




CDirectiveFunction::CDirectiveFunction(const std::wstring& name)
{
	this->label = new CAssemblerLabel(name);
	this->content = nullptr;
	this->start = this->end = 0;
}

CDirectiveFunction::~CDirectiveFunction()
{
	delete label;
	delete content;
}

bool CDirectiveFunction::Validate()
{
	start = g_fileManager->getVirtualAddress();

	label->applyFileInfo();
	bool result = label->Validate();

	content->applyFileInfo();
	if (content->Validate())
		result = true;

	end = g_fileManager->getVirtualAddress();
	return result;
}

void CDirectiveFunction::Encode() const
{
	label->applyFileInfo();
	label->Encode();

	content->applyFileInfo();
	content->Encode();
}

void CDirectiveFunction::writeTempData(TempData& tempData) const
{
	label->writeTempData(tempData);
	content->writeTempData(tempData);
}

void CDirectiveFunction::writeSymData(SymbolData& symData) const
{
	symData.startFunction(start);
	label->writeSymData(symData);
	content->writeSymData(symData);
	symData.endFunction(end);
}
