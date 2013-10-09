#include "stdafx.h"
#include "Commands/CDirectiveFill.h"
#include "Core/Common.h"

CDirectiveFill::CDirectiveFill()
{
	Size = 0;
	Byte = 0;
}

bool CDirectiveFill::Load(ArgumentList &Args)
{
	initExpression(SizeExpression,Args[0].text);

	if (Args.size() == 2)	// includes fill byte
	{
		FillByte = true;
		initExpression(ByteExpression,Args[1].text);
	} else {
		FillByte = false;
		Byte = 0;
	}

	return true;
}

bool CDirectiveFill::Validate()
{
	int NewSize;

	RamPos = Global.RamPos;
	if (evalExpression(SizeExpression,NewSize,true) == false)
		return false;

	if (FillByte == true)
	{
		if (evalExpression(ByteExpression,Byte,true) == false)
			return false;
	}

	Global.RamPos += NewSize;

	if (Size != NewSize)
	{
		Size = NewSize;
		return true;
	} else {
		return false;
	}
}

void CDirectiveFill::Encode()
{
	unsigned char ByteBuffer[128];

	memset(ByteBuffer,Byte,Size > 128 ? 128 : Size);
	int n = Size;
	while (n > 128)
	{
		Global.Output.write(ByteBuffer,128);
		n -= 128;
	}
	Global.Output.write(ByteBuffer,n);
}

void CDirectiveFill::writeTempData(TempData& tempData)
{
	tempData.writeLine(RamPos,formatString(L".fill 0x%08X,0x%02X",Size,Byte));
}

void CDirectiveFill::writeSymData(SymbolData& symData)
{
	symData.addSymbol(RamPos,formatString(L".byt:%04X",Size));
}