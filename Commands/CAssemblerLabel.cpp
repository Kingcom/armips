#include "stdafx.h"
#include "Commands/CAssemblerLabel.h"
#include "Core/Common.h"
#include "Util/Util.h"
#include "Core/FileManager.h"
#include "Archs/ARM/Arm.h"

CAssemblerLabel::CAssemblerLabel(const std::wstring& name, u64 RamPos, int Section, bool constant)
{
	this->labelname = name;
	this->labelvalue = RamPos;
	this->constant = constant;
	this->label = NULL;
}

bool CAssemblerLabel::Validate()
{
	if (label == NULL)
	{
		label = Global.symbolTable.getLabel(this->labelname, FileNum, Global.Section);
		if (label == NULL)
		{
			Logger::printError(Logger::Error, L"Invalid label name \"%s\"", this->labelname);
			return false;
		}

		if (label->isDefined())
		{
			Logger::printError(Logger::Error, L"Label \"%s\" already defined", this->labelname);
			label = NULL;
			return false;
		}

		if (label->getUpdateInfo())
		{
			if (Arch == &Arm && Arm.GetThumbMode())
				label->setInfo(1);
			else
				label->setInfo(0);
		}

		label->setValue(labelvalue);
		label->setDefined(true);
		if (Global.symbolTable.isLocalSymbol(this->labelname) == false)
			Global.Section++;
		return true;
	}
	if (Global.symbolTable.isLocalSymbol(this->labelname) == false)
		Global.Section++;
	if (constant == false && label->getValue() != g_fileManager->getVirtualAddress())
	{
		label->setValue(g_fileManager->getVirtualAddress());
		return true;
	}
	return false;
}

void CAssemblerLabel::Encode()
{
	if (Global.symbolTable.isLocalSymbol(this->labelname) == false)
		Global.Section++;
}

void CAssemblerLabel::writeTempData(TempData& tempData)
{
	tempData.writeLine(label->getValue(),formatString(L"%s:",label->getName()));
}

void CAssemblerLabel::writeSymData(SymbolData& symData)
{
	symData.addLabel(label->getValue(),label->getName());
}




CDirectiveFunction::CDirectiveFunction(const std::wstring& name, int Section)
{
	if (!name.empty())
		label = new CAssemblerLabel(name,g_fileManager->getVirtualAddress(),Section,false);
	else
		label = NULL;
}

CDirectiveFunction::~CDirectiveFunction()
{
	delete label;
}

bool CDirectiveFunction::Validate()
{
	if (label == NULL)
		return false;
	return label->Validate();
}

void CDirectiveFunction::writeTempData(TempData& tempData)
{
	if (label != NULL)
		label->writeTempData(tempData);
}

void CDirectiveFunction::writeSymData(SymbolData& symData)
{
	if (label != NULL)
	{
		Global.symData.startFunction(g_fileManager->getVirtualAddress());
		label->writeSymData(symData);
	} else {
		Global.symData.endFunction(g_fileManager->getVirtualAddress());
	}
}
