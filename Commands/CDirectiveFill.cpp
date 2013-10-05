#include "stdafx.h"
#include "Commands/CDirectiveFill.h"
#include "Core/Common.h"

CDirectiveFill::CDirectiveFill()
{
	Size = 0;
	Byte = 0;
}

bool CDirectiveFill::Load(CArgumentList &Args)
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

	if (Args.GetCount() == 2)	// mit fill byte
	{
		FillByte = true;
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
		ByteExpression.Load(List);
	} else {
		FillByte = false;
		Byte = 0;
	}

	return true;
}

bool CDirectiveFill::Validate()
{
	CStringList List;
	int NewSize;

	RamPos = Global.RamPos;

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

	if (FillByte == true)
	{
		if (ParsePostfix(ByteExpression,&List,Byte) == false)
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
		Global.Output.Write(ByteBuffer,128);
		n -= 128;
	}
	Global.Output.Write(ByteBuffer,n);

	if (Global.SymData.Write == true)
	{
		fprintf(Global.SymData.Handle,"%08X .byt:%04X\n",RamPos,Size);
	}
}

void CDirectiveFill::WriteTempData(FILE *&Output)
{
	char str[256];

	sprintf_s(str,256,".fill 0x%08X,0x%02X",Size,Byte);
	WriteToTempData(Output,str,RamPos);
}