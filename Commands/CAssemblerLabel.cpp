#include "StdAfx.h"
#include "Commands/CAssemblerLabel.h"
#include "Core/Common.h"

CAssemblerLabel::CAssemblerLabel(char* str, int RamPos, int Section, bool constant)
{
	strcpy(LabelName,str);
	LabelRamPos = RamPos;
	LabelSection = Section;

	switch (Global.Labels.AddLabel(LabelName,LabelRamPos,LabelSection,FileNum,true))
	{
	case LABEL_ALREADYDEFINED:
		PrintError(ERROR_ERROR,"Label \"%s\" already defined",LabelName);
		break;
	case LABEL_INVALIDNAME:
		PrintError(ERROR_ERROR,"Invalid label name \"%s\"",LabelName);
		break;
	}

	LabelConstant = constant;
	if (Global.Labels.IsLocal(LabelName) == false) Global.Section++;
}

bool CAssemblerLabel::Validate()
{
	if (Global.RamPos != LabelRamPos && LabelConstant == false)
	{
		LabelRamPos = Global.RamPos;
		Global.Labels.UpdateLabelValue(LabelName,LabelSection,FileNum,LabelRamPos);
		return true;
	} else {
		return false;
	}
}

void CAssemblerLabel::WriteTempData(FILE*& Output)
{
	char str[256];

	sprintf(str,"%s:",LabelName);
	WriteToTempData(Output,str,LabelRamPos);
}

void CAssemblerLabel::Encode()
{
	if (Global.SymData.Write == true)
	{
		fprintf(Global.SymData.Handle,"%08X %s\n",LabelRamPos,LabelName);
	}
}
