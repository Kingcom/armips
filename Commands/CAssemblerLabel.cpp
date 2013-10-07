#include "StdAfx.h"
#include "Commands/CAssemblerLabel.h"
#include "Core/Common.h"
#include "Util/Util.h"

CAssemblerLabel::CAssemblerLabel(const std::wstring& name, int RamPos, int Section, bool constant)
{
	label = Global.symbolTable.getLabel(name,FileNum,Section);
	if (label == NULL)
	{
		PrintError(ERROR_ERROR,"Invalid label name \"%s\"",convertWStringToUtf8(name).c_str());
		return;
	}

	if (label->isDefined())
	{
		PrintError(ERROR_ERROR,"Label \"%s\" already defined",convertWStringToUtf8(name).c_str());
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

void CAssemblerLabel::WriteTempData(FILE*& Output)
{
	char str[256];

	sprintf(str,"%s:",convertWStringToUtf8(label->getName()).c_str());
	WriteToTempData(Output,str,label->getValue());
}

void CAssemblerLabel::Encode()
{
	if (Global.SymData.Write == true)
	{
		fprintf(Global.SymData.Handle,"%08X %s\n",label->getValue(),convertWStringToUtf8(label->getName()).c_str());
	}
}
