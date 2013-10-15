#include "stdafx.h"
#include "Commands/CDirectiveData.h"
#include "Core/Common.h"
#include "Core/MathParser.h"
#include "Core/FileManager.h"

CDirectiveData::CDirectiveData(ArgumentList& Args, int SizePerUnit, bool asc)
{
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
		Logger::printError(Logger::Error,L"Invalid data unit size %d",SizePerUnit);
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
	g_fileManager->advanceMemory(SpaceNeeded);
}

CDirectiveData::~CDirectiveData()
{
	free(Entries);
	delete[] ExpData;
}

bool CDirectiveData::Validate()
{
	RamPos = g_fileManager->getVirtualAddress();

	int num;
	for (int i = 0; i < TotalAmount; i++)
	{
		if (Entries[i].String == false)
		{
			if (evalExpression(ExpData[Entries[i].num],num,true) == false)
				return false;
			g_fileManager->advanceMemory(UnitSize);
		} else {
			g_fileManager->advanceMemory(StrData.GetLen(Entries[i].num)*UnitSize);
		}
	}

	return false;
}

void CDirectiveData::Encode()
{
	int num;
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
				g_fileManager->write(&num,UnitSize);
			}
			totalsize += len*UnitSize;
		} else {
			if (evalExpression(ExpData[Entries[i].num],num) == false)
				return;
			totalsize += UnitSize;
			g_fileManager->write(&num,UnitSize);
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
		symData.addData(RamPos,SpaceNeeded,SymbolData::DataAscii);
	} else {
		switch (UnitSize)
		{
		case 1:
			symData.addData(RamPos,SpaceNeeded,SymbolData::Data8);
			break;
		case 2:
			symData.addData(RamPos,SpaceNeeded,SymbolData::Data16);
			break;
		case 4:
			symData.addData(RamPos,SpaceNeeded,SymbolData::Data32);
			break;
		}
	}
}