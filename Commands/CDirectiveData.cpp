#include "stdafx.h"
#include "Commands/CDirectiveData.h"
#include "Core/Common.h"
#include "Core/FileManager.h"

CDirectiveData::CDirectiveData(ArgumentList& Args, size_t SizePerUnit, bool asc)
{
	// temporary solution until ArgumentList consists of expressions
	for (int i = 0; i < Args.size(); i++)
	{
		std::wstring text = Args[i].text;
		if (Args[i].isString)
			text = L"\"" + text + L"\"";

		Expression exp;
		if (exp.load(text) == false)
		{
			Logger::printError(Logger::Error,L"Invalid expression");
			return;
		}

		entries.push_back(exp);
	}

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
}

CDirectiveData::~CDirectiveData()
{

}

bool CDirectiveData::Validate()
{
	RamPos = g_fileManager->getVirtualAddress();

	for (size_t i = 0; i < entries.size(); i++)
	{
		ExpressionValue value = entries[i].evaluate();
		if (!value.isValid())
		{
			Logger::queueError(Logger::Error,L"Invalid expression");
			return false;
		}

		if (value.isInt())
			g_fileManager->advanceMemory(UnitSize);
		else if (value.isString())
			g_fileManager->advanceMemory(value.strValue.size()*UnitSize);
		else
			Logger::queueError(Logger::Error,L"Invalid expression type");
	}

	return false;
}

void CDirectiveData::Encode()
{
	;

	for (size_t i = 0; i < entries.size(); i++)
	{
		ExpressionValue value = entries[i].evaluate();

		if (value.isString())
		{
			for (size_t l = 0; l < value.strValue.size(); l++)
			{
				u64 num = value.strValue[l];
				g_fileManager->write(&num,UnitSize);
				SpaceNeeded += UnitSize;
			}
		} else if (value.isInt())
		{
			// swap endianess if the output is big endian
			u64 num = value.intValue;
			if (Arch->getEndianness() == Endianness::Big)
			{
				switch (UnitSize)
				{
				case 2:
					num = swapEndianness16((u16)num);
					break;
				case 4:
					num = swapEndianness32((u32)num);
					break;
				}
			}

			g_fileManager->write(&num,UnitSize);
			SpaceNeeded += UnitSize;
		}
	}
}


void CDirectiveData::writeTempData(TempData& tempData)
{
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

	for (size_t i = 0; i < entries.size(); i++)
	{
		ExpressionValue value = entries[i].evaluate();

		if (value.isString())
		{
			for (size_t l = 0; l < value.strValue.size(); l++)
			{
				result += formatString(L"0x%0*X,",UnitSize*2,value.strValue[l]);
			}
		} else if (value.isInt())
		{
			result += formatString(L"0x%0*X,",UnitSize*2,value.intValue);
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