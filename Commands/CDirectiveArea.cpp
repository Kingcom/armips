#include "stdafx.h"
#include "Commands/CDirectiveArea.h"
#include "Core/Common.h"

CDirectiveArea::CDirectiveArea()
{
	Start = true;
	UseFill = false;
	Size = 0;
}

bool CDirectiveArea::LoadStart(CArgumentList &Args)
{
	CStringList List;

	if (ConvertInfixToPostfix(Args.GetEntry(0),List) == false)
	{
		PrintError(ERROR_ERROR,"Invalid expression \"%\"",Args.GetEntry(0));
		return false;
	}
	if (CheckPostfix(List,true) == false)
	{
		PrintError(ERROR_ERROR,"Invalid expression \"%\"",Args.GetEntry(0));
		return false;
	}
	SizeExpression.Load(List);
	Start = true;

	if (Args.GetCount() == 2)
	{
		if (ConvertInfixToPostfix(Args.GetEntry(1),List) == false)
		{
			PrintError(ERROR_ERROR,"Invalid expression \"%\"",Args.GetEntry(1));
			return false;
		}
		if (CheckPostfix(List,true) == false)
		{
			PrintError(ERROR_ERROR,"Invalid expression \"%\"",Args.GetEntry(1));
			return false;
		}
		FillExpression.Load(List);
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
		if (ParsePostfix(SizeExpression,&List,NewSize) == false)
		{
			if (List.GetCount() == 0)
			{
				QueueError(ERROR_ERROR,"Invalid expression");
			} else {
				for (int l = 0; l < List.GetCount(); l++)
				{
					QueueError(ERROR_ERROR,List.GetEntry(l));
				}
			}
			return false;
		}
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