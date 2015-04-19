#include "stdafx.h"
#include "Commands/CDirectiveArea.h"
#include "Core/Common.h"
#include "Core/FileManager.h"
#include <algorithm>

CDirectiveArea::CDirectiveArea()
{
	Start = true;
	fillValue = -1;
	Size = 0;
}

bool CDirectiveArea::LoadStart(ArgumentList &Args)
{
	if (SizeExpression.load(Args[0].text) == false)
		return false;
	Start = true;

	if (Args.size() == 2)
	{
		if (FillExpression.load(Args[1].text) == false)
			return false;
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
	size_t NewSize;

	RamPos = g_fileManager->getVirtualAddress();

	if (Start == true)
	{
		if (SizeExpression.evaluateInteger(NewSize) == false)
			return false;

		if (FillExpression.isLoaded())
		{
			if (FillExpression.evaluateInteger(fillValue) == false)
				return false;
		}

		Global.areaData.startArea(RamPos,NewSize,FileNum,FileLine,fillValue);

		if (Size != NewSize)
		{
			Size = NewSize;
			return true;
		} else {
			return false;
		}
	} else {
		if (Global.areaData.getEntryCount() != 0)
		{
			fillValue = Global.areaData.getCurrentFillValue();
			Size = (size_t) (Global.areaData.getCurrentMaxAddress()-g_fileManager->getVirtualAddress());

			if (fillValue != -1)
				g_fileManager->seekVirtual(Global.areaData.getCurrentMaxAddress());
		}

		Global.areaData.endArea();
		return false;
	}
}

void CDirectiveArea::Encode()
{
	if (Start == false && fillValue != -1)
	{
		unsigned char buffer[64];
		memset(buffer,fillValue,64);

		while (Size > 0)
		{
			size_t part = std::min<size_t>(64,Size);
			g_fileManager->write(buffer,part);
			Size -= part;
		}
	}
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