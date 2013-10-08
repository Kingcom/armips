#include "stdafx.h"
#include "Commands/CDirectiveArea.h"
#include "Core/Common.h"

CDirectiveArea::CDirectiveArea()
{
	Start = true;
	UseFill = false;
	Size = 0;
}

bool CDirectiveArea::LoadStart(ArgumentList &Args)
{
	CStringList List;

	if (initExpression(SizeExpression,Args[0].text) == false)
		return false;
	Start = true;

	if (Args.size() == 2)
	{
		if (initExpression(FillExpression,Args[1].text) == false)
			return false;
		UseFill = true;
	}

	return true;
}

bool CDirectiveArea::LoadEnd()
{
	Start = false;
	return true;
}

bool CDirectiveArea::Validate()
{
	CStringList List;
	int NewSize;

	RamPos = Global.RamPos;

	if (Start == true)
	{
		if (evalExpression(SizeExpression,NewSize,true) == false)
			return false;
		Global.AreaData.Entries[Global.AreaData.EntryCount].FileNum = FileNum;
		Global.AreaData.Entries[Global.AreaData.EntryCount].LineNumber = FileLine;
		Global.AreaData.Entries[Global.AreaData.EntryCount++].MaxRamPos = RamPos+NewSize;

		if (Size != NewSize)
		{
			Size = NewSize;
			return true;
		} else {
			return false;
		}
	} else {
		if (Global.AreaData.EntryCount-- == 0)
		{
			QueueError(ERROR_ERROR,"No active area");
			Global.AreaData.EntryCount = 0;
		}
		return false;
	}
}

void CDirectiveArea::Encode()
{
	return;
}

void CDirectiveArea::WriteTempData(FILE *&Output)
{
	char str[256];

	if (Start == true)
	{
		sprintf_s(str,256,".area 0x%08X",Size);
	} else {
		sprintf_s(str,256,".endarea");
	}

	WriteToTempData(Output,str,RamPos);
}