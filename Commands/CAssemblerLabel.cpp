#include "StdAfx.h"
#include "Commands/CAssemblerLabel.h"
#include "Core/Common.h"
#include "Util/Util.h"

CAssemblerLabel::CAssemblerLabel(const std::wstring& name, int RamPos, int Section, bool constant)
{
	label = Global.symbolTable.getLabel(name,FileNum,Section);
	if (label == NULL)
	{
		PrintError(ERROR_ERROR,"Invalid label name \"%ls\"",name.c_str());
		return;
	}

	if (label->isDefined())
	{
		PrintError(ERROR_ERROR,"Label \"%ls\" already defined",name.c_str());
		return;
	}

	label->setValue(RamPos);
	label->setDefined(true);
	this->constant = constant;

	if (Global.symbolTable.isLocalSymbol(name) == false)
		Global.Section++;
}

bool CAssemblerLabel::Validate()
{
	if (constant == false && label->getValue() != Global.RamPos)
	{
		label->setValue(Global.RamPos);
		return true;
	}
	
	return false;
}

void CAssemblerLabel::writeTempData(TempData& tempData)
{
	tempData.writeLine(label->getValue(),formatString(L"%ls:",label->getName().c_str()));
}

void CAssemblerLabel::Encode()
{
	if (Global.SymData.Write == true)
	{
		fprintf(Global.SymData.Handle,"%08X %ls\n",label->getValue(),label->getName().c_str());
	}
}
