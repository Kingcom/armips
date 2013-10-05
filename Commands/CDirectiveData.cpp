#include "stdafx.h"
#include "Commands/CDirectiveData.h"
#include "Core/Common.h"
#include "Core/MathParser.h"

CDirectiveData::CDirectiveData(CArgumentList& Args, int SizePerUnit, bool asc)
{
	CStringList List;

	TotalAmount = Args.GetCount();
	StrAmount = 0;
	ExpAmount = 0;
	for (int i = 0; i < TotalAmount; i++)
	{
		if (Args.IsString(i) == true)
		{
			StrAmount++;
		} else {
			ExpAmount++;
		}
	}

	Entries = (tDirectiveDataEntry*) malloc(TotalAmount*sizeof(tDirectiveDataEntry));
	ExpData = new CExpressionCommandList[ExpAmount];
	
	switch (SizePerUnit)
	{
	case 1: case 2: case 4:
		UnitSize = SizePerUnit;
		ascii = asc;
		break;
	default:
		PrintError(ERROR_ERROR,"Invalid data unit size %d",SizePerUnit);
		return;
	}

	int ExpNum = 0;
	SpaceNeeded = 0;
	for (int i = 0; i < TotalAmount; i++)
	{
		if (Args.IsString(i) == true)
		{
			int len = strlen(Args.GetEntry(i));
			Entries[i].String = true;
			Entries[i].num = StrData.GetCount();
			StrData.AddEntry((unsigned char*)Args.GetEntry(i),len);
			SpaceNeeded += len*UnitSize;
		} else {
			Entries[i].String = false;
			Entries[i].num = ExpNum;

			if (ConvertInfixToPostfix(Args.GetEntry(i),List) == false)
			{
				PrintError(ERROR_ERROR,"Invalid number or label in expression \"%s\"",Args.GetEntry(i));
				return;
			}
			if (CheckPostfix(List,true) == false)
			{
				PrintError(ERROR_ERROR,"Invalid expression \"%s\"",Args.GetEntry(i));
				return;
			}
			ExpData[ExpNum++].Load(List);
			SpaceNeeded += UnitSize;
		}
	}
	Global.RamPos += SpaceNeeded;
}

CDirectiveData::~CDirectiveData()
{
	free(Entries);
	delete[] ExpData;
}

bool CDirectiveData::Validate()
{
	CStringList List;

	RamPos = Global.RamPos;

	int num;
	for (int i = 0; i < TotalAmount; i++)
	{
		if (Entries[i].String == false)
		{
			if (ParsePostfix(ExpData[Entries[i].num],&List,num) == false)
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
			Global.RamPos += UnitSize;
		} else {
			Global.RamPos += StrData.GetLen(Entries[i].num)*UnitSize;
		}
	}

	return false;
}

void CDirectiveData::Encode()
{
	CStringList List;
	int num;

	Global.RamPos = RamPos;
	int totalsize = 0;

	for (int i = 0; i < TotalAmount; i++)
	{
		if (Entries[i].String == true)
		{
			unsigned char* Data = StrData.GetEntry(Entries[i].num);
			int len = StrData.GetLen(Entries[i].num);
			for (int i = 0; i < len; i++)
			{
				num = Data[i];
				Global.Output.Write(&num,UnitSize);
			}
			Global.RamPos += len*UnitSize;
			totalsize += len*UnitSize;
		} else {
			if (ParsePostfix(ExpData[Entries[i].num],&List,num) == false)
			{
				if (List.GetCount() == 0)
				{
					PrintError(ERROR_ERROR,"Invalid expression");
				} else {
					for (int l = 0; l < List.GetCount(); l++)
					{
						PrintError(ERROR_ERROR,List.GetEntry(l));
					}
				}
				return;
			}
			Global.RamPos += UnitSize;
			totalsize += UnitSize;
			Global.Output.Write(&num,UnitSize);
		}
	}

	if (Global.SymData.Write == true)
	{
		if (ascii == true)
		{
			fprintf(Global.SymData.Handle,"%08X .asc:%04X\n",RamPos,totalsize);
		} else {
			switch (UnitSize)
			{
			case 1: fprintf(Global.SymData.Handle,"%08X .byt:%04X\n",RamPos,totalsize);
				break;
			case 2: fprintf(Global.SymData.Handle,"%08X .wrd:%04X\n",RamPos,totalsize);
				break;
			case 4: fprintf(Global.SymData.Handle,"%08X .dbl:%04X\n",RamPos,totalsize);
				break;
			}
		}
	}
}


void CDirectiveData::WriteTempData(FILE*& Output)
{
	CStringList List;
	char str[2048];
	int pos = 0;
	int num;

/*	if (Global.SymData.Write == true)
	{
		switch (UnitSize)
		{
		case 1:
			fprintf(Global.SymData.Handle,".byt:%04X\n",SpaceNeeded);
			break;
		case 2:
			fprintf(Global.SymData.Handle,".wrd:%04X\n",SpaceNeeded);
			break;
		case 4:
			fprintf(Global.SymData.Handle,".dbl:%04X\n",SpaceNeeded);
			break;
		}
	}*/

	switch (UnitSize)
	{
	case 1:
		pos = sprintf(str,".byte ");
		break;
	case 2:
		pos = sprintf(str,".halfword ");
		break;
	case 4:
		pos = sprintf(str,".word ");
		break;
	}

	Global.RamPos = RamPos;

	for (int i = 0; i < TotalAmount; i++)
	{
		if (Entries[i].String == true)
		{
			unsigned char* Data = StrData.GetEntry(Entries[i].num);
			int len = StrData.GetLen(Entries[i].num);
			for (int i = 0; i < len; i++)
			{
				pos += sprintf(&str[pos],"0x%0*X,",UnitSize*2,Data[i]);
			}
			Global.RamPos += len*UnitSize;
		} else {
			if (ParsePostfix(ExpData[Entries[i].num],&List,num) == false)
			{
				if (List.GetCount() == 0)
				{
					PrintError(ERROR_ERROR,"Invalid expression");
				} else {
					for (int l = 0; l < List.GetCount(); l++)
					{
						PrintError(ERROR_ERROR,List.GetEntry(l));
					}
				}
				return;
			}
			Global.RamPos += UnitSize;
			pos += sprintf(&str[pos],"0x%0*X,",UnitSize*2,num);
		}
	}
	str[pos-1] = 0;
	WriteToTempData(Output,str,RamPos);
}