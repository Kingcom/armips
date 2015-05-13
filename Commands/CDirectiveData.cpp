#include "stdafx.h"
#include "Commands/CDirectiveData.h"
#include "Core/Common.h"
#include "Core/FileManager.h"

//
// TableCommand
//

TableCommand::TableCommand(const std::wstring& fileName, TextFile::Encoding encoding)
{
	if (fileExists(fileName) == false)
	{
		Logger::printError(Logger::Error,L"Table file \"%s\" does not exist",fileName);
		return;
	}

	if (table.load(fileName,encoding) == false)
	{
		Logger::printError(Logger::Error,L"Invalid table file \"%s\"",fileName);
		return;
	}
}

bool TableCommand::Validate()
{
	Global.Table = table;
	return false;
}


//
// CDirectiveData
//

CDirectiveData::CDirectiveData()
{
	mode = EncodingMode::Invalid;
	writeTermination = false;
}

CDirectiveData::~CDirectiveData()
{

}

void CDirectiveData::setNormal(std::vector<Expression>& entries, size_t unitSize, bool ascii)
{
	switch (unitSize)
	{
	case 1:
		if (ascii)
			this->mode = EncodingMode::Ascii;
		else
			this->mode = EncodingMode::U8;
		break;
	case 2:
		this->mode = EncodingMode::U16;
		break;
	case 4:
		this->mode = EncodingMode::U32;
		break;
	default:
		Logger::printError(Logger::Error,L"Invalid data unit size %d",unitSize);
		return;
	}
	
	this->entries = entries;
	this->writeTermination = false;
}

void CDirectiveData::setSjis(std::vector<Expression>& entries, bool terminate)
{
	this->mode = EncodingMode::Sjis;
	this->entries = entries;
	this->writeTermination = terminate;
}

void CDirectiveData::setCustom(std::vector<Expression>& entries, bool terminate)
{
	this->mode = EncodingMode::Custom;
	this->entries = entries;
	this->writeTermination = terminate;
}

size_t CDirectiveData::getUnitSize() const
{
	switch (mode)
	{
	case EncodingMode::U8:
	case EncodingMode::Ascii:
	case EncodingMode::Sjis:
	case EncodingMode::Custom:
		return 1;
	case EncodingMode::U16:
		return 2;
	case EncodingMode::U32:
		return 4;
	}

	return 0;
}

void CDirectiveData::encodeCustom(EncodingTable& table)
{
	data.clear();
	for (size_t i = 0; i < entries.size(); i++)
	{
		ExpressionValue value = entries[i].evaluate();
		if (!value.isValid())
		{
			Logger::queueError(Logger::Error,L"Invalid expression");
			continue;
		}
		
		if (value.isInt())
		{
			data.appendByte((u8)value.intValue);
		} else if (value.isString())
		{
			ByteArray encoded = table.encodeString(value.strValue,false);
			data.append(encoded);
		} else {
			Logger::queueError(Logger::Error,L"Invalid expression type");
		}
	}

	if (writeTermination)
	{
		ByteArray encoded = table.encodeTermination();
		data.append(encoded);
	}
}

void CDirectiveData::encodeSjis()
{
	static EncodingTable sjisTable;
	if (sjisTable.isLoaded() == false)
	{
		unsigned char hexBuffer[2];
		
		sjisTable.setTerminationEntry((unsigned char*)"\0",1);

		for (unsigned short SJISValue = 0x0020; SJISValue < 0x0100; SJISValue++)
		{
			wchar_t unicodeValue = sjisToUnicode(SJISValue);
			if (unicodeValue != 0xFFFF)
			{
				hexBuffer[0] = SJISValue & 0xFF;
				sjisTable.addEntry(hexBuffer, 1, unicodeValue);
			}
		}
		for (unsigned short SJISValue = 0x8100; SJISValue < 0xEF00; SJISValue++)
		{
			wchar_t unicodeValue = sjisToUnicode(SJISValue);
			if (unicodeValue != 0xFFFF)
			{
				hexBuffer[0] = (SJISValue >> 8) & 0xFF;
				hexBuffer[1] = SJISValue & 0xFF;
				sjisTable.addEntry(hexBuffer, 2, unicodeValue);
			}
		}
	}

	encodeCustom(sjisTable);
}

void CDirectiveData::encodeNormal()
{
	size_t unitSize = getUnitSize();

	data.clear();
	for (size_t i = 0; i < entries.size(); i++)
	{
		ExpressionValue value = entries[i].evaluate();
		if (!value.isValid())
		{
			Logger::queueError(Logger::Error,L"Invalid expression");
			continue;
		}

		if (value.isString())
		{
			bool hadNonAscii = false;
			for (size_t l = 0; l < value.strValue.size(); l++)
			{
				u64 num = value.strValue[l];
				data.append(&num,unitSize);

				if (num >= 0x80 && hadNonAscii == false)
				{
					Logger::printError(Logger::Warning,L"Non-ASCII character in data directive. Use .string instead");
					hadNonAscii = true;
				}
			}
		} else if (value.isInt())
		{
			// swap endianess if the output is big endian
			u64 num = value.intValue;
			if (Arch->getEndianness() == Endianness::Big)
			{
				switch (unitSize)
				{
				case 2:
					num = swapEndianness16((u16)num);
					break;
				case 4:
					num = swapEndianness32((u32)num);
					break;
				}
			}

			data.append(&num,unitSize);
		} else {
			Logger::queueError(Logger::Error,L"Invalid expression type");
		}
	}
}

bool CDirectiveData::Validate()
{
	position = g_fileManager->getVirtualAddress();

	size_t oldSize = data.size();
	switch (mode)
	{
	case EncodingMode::U8:
	case EncodingMode::U16:
	case EncodingMode::U32:
		encodeNormal();
		break;
	case EncodingMode::Sjis:
		encodeSjis();
		break;
	case EncodingMode::Custom:
		encodeCustom(Global.Table);
		break;
	default:
		Logger::queueError(Logger::Error,L"Invalid encoding type");
		break;
	}

	g_fileManager->advanceMemory(data.size());
	return oldSize != data.size();
}

void CDirectiveData::Encode() const
{
	g_fileManager->write(data.data(),data.size());
}

void CDirectiveData::writeTempData(TempData& tempData) const
{
	size_t size = (getUnitSize()*2+3)*data.size()+20;
	wchar_t* str = new wchar_t[size];
	wchar_t* start = str;

	switch (mode)
	{
	case EncodingMode::U8:
	case EncodingMode::Ascii:
	case EncodingMode::Sjis:
	case EncodingMode::Custom:
		str += swprintf(str,20,L".byte ");
		break;
	case EncodingMode::U16:
		str += swprintf(str,20,L".halfword ");
		break;
	case EncodingMode::U32:
		str += swprintf(str,20,L".word ");
		break;
	}

	size_t unitSize = getUnitSize();
	for (size_t i = 0; i < data.size(); i++)
	{
		str += swprintf(str,20,L"0x%0*X,",unitSize*2,data[i]);
	}

	*(str-1) = 0;
	tempData.writeLine(position,start);
	delete[] start;
}

void CDirectiveData::writeSymData(SymbolData& symData) const
{
	switch (mode)
	{
	case EncodingMode::Ascii:
		symData.addData(position,data.size(),SymbolData::DataAscii);
		break;
	case EncodingMode::U8:
	case EncodingMode::Sjis:
	case EncodingMode::Custom:
		symData.addData(position,data.size(),SymbolData::Data8);
		break;
	case EncodingMode::U16:
		symData.addData(position,data.size(),SymbolData::Data16);
		break;
	case EncodingMode::U32:
		symData.addData(position,data.size(),SymbolData::Data32);
		break;
	}
}
