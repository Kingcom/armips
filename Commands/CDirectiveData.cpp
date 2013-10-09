#include "stdafx.h"
#include "Commands/CDirectiveData.h"
#include "Core/Common.h"
#include "Core/MathParser.h"

CDirectiveData::CDirectiveData(ArgumentList& Args, int SizePerUnit, bool asc)
{
	CStringList List;

	TotalAmount = Args.size();
	StrAmount = 0;
	ExpAmount = 0;
	for (int i = 0; i < TotalAmount; i++)
	{
		if (Args[i].isString == true)
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
		if (Args[i].isString == true)
		{
			std::string tt = convertWStringToUtf8(Args[i].text);
			char* t = (char*) tt.c_str();

			int len = strlen(t);
			Entries[i].String = true;
			Entries[i].num = StrData.GetCount();
			StrData.AddEntry((unsigned char*)t,len);
			SpaceNeeded += len*UnitSize;
		} else {
			Entries[i].String = false;
			Entries[i].num = ExpNum;

			if (initExpression(ExpData[ExpNum++],Args[i].text) == false)
				return;

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
			if (evalExpression(ExpData[Entries[i].num],num,true) == false)
				return false;
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
				Global.Output.write(&num,UnitSize);
			}
			Global.RamPos += len*UnitSize;
			totalsize += len*UnitSize;
		} else {
			if (evalExpression(ExpData[Entries[i].num],num) == false)
				return;
			Global.RamPos += UnitSize;
			totalsize += UnitSize;
			Global.Output.write(&num,UnitSize);
		}
	}

	SpaceNeeded = totalsize;
}


void CDirectiveData::writeTempData(TempData& tempData)
{
	int num;

	std::wstring result;
	switch (UnitSize)
	{
	case 1:
		result = L".byte ";
		break;
	case 2:
		result = L".halfword ";
		break;
	case 4:
		result = L".word ";
		break;
	}

	for (int i = 0; i < TotalAmount; i++)
	{
		if (Entries[i].String == true)
		{
			unsigned char* Data = StrData.GetEntry(Entries[i].num);
			int len = StrData.GetLen(Entries[i].num);
			for (int i = 0; i < len; i++)
			{
				result += formatString(L"0x%0*X,",UnitSize*2,Data[i]);
			}
		} else {
			if (evalExpression(ExpData[Entries[i].num],num) == false)
				return;
			result += formatString(L"0x%0*X,",UnitSize*2,num);
		}
	}

	result.pop_back();
	tempData.writeLine(RamPos,result);
}

void CDirectiveData::writeSymData(SymbolData& symData)
{
	if (ascii == true)
	{
		symData.addSymbol(RamPos,formatString(L".asc:%04X",SpaceNeeded));
	} else {
		switch (UnitSize)
		{
		case 1:
			symData.addSymbol(RamPos,formatString(L".byt:%04X",SpaceNeeded));
			break;
		case 2:
			symData.addSymbol(RamPos,formatString(L".wrd:%04X",SpaceNeeded));
			break;
		case 4:
			symData.addSymbol(RamPos,formatString(L".dbl:%04X",SpaceNeeded));
			break;
		}
	}
}