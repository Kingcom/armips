#include "stdafx.h"
#include "Commands/CDirectiveArea.h"
#include "Core/Common.h"
#include "Core/FileManager.h"

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

	RamPos = g_fileManager->getVirtualAddress();

	if (Start == true)
	{
		if (evalExpression(SizeExpression,NewSize,true) == false)
			return false;
		Global.areaData.startArea(RamPos,NewSize,FileNum,FileLine);

		if (Size != NewSize)
		{
			Size = NewSize;
			return true;
		} else {
			return false;
		}
	} else {
		Global.areaData.endArea();
		return false;
	}
}

void CDirectiveArea::Encode()
{
	return;
}

void CDirectiveArea::writeTempData(TempData& tempData)
{
	if (Start == true)
	{
		tempData.writeLine(RamPos,formatString(L".area 0x%08X",Size));
	} else {
		tempData.writeLine(RamPos,L".endarea");
	}
}