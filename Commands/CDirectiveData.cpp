#include "Commands/CDirectiveData.h"

#include <cinttypes>
#include "Archs/Architecture.h"
#include "Core/Common.h"
#include "Core/FileManager.h"
#include "Core/Misc.h"
#include "Core/SymbolData.h"
#include "Util/FileSystem.h"
#include "Util/Util.h"

//
// TableCommand
//

TableCommand::TableCommand(const fs::path& fileName, TextFile::Encoding encoding)
{
	auto fullName = getFullPathName(fileName);

	if (!fs::exists(fullName))
	{
		Logger::printError(Logger::Error, "Table file \"%s\" does not exist", fileName.u8string());
		return;
	}

	if (!table.load(fullName,encoding))
	{
		Logger::printError(Logger::Error, "Invalid table file \"%s\"",fileName.u8string());
		return;
	}
}

bool TableCommand::Validate(const ValidateState &state)
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
	endianness = Architecture::current().getEndianness();
}

CDirectiveData::~CDirectiveData()
{

}

void CDirectiveData::setNormal(std::vector<Expression>& entries, size_t unitSize)
{
	switch (unitSize)
	{
	case 1:
		this->mode = EncodingMode::U8;
		break;
	case 2:
		this->mode = EncodingMode::U16;
		break;
	case 4:
		this->mode = EncodingMode::U32;
		break;
	case 8:
		this->mode = EncodingMode::U64;
		break;
	default:
		Logger::printError(Logger::Error, "Invalid data unit size %d",unitSize);
		return;
	}
	
	this->entries = entries;
	this->writeTermination = false;
	normalData.reserve(entries.size());
}

void CDirectiveData::setFloat(std::vector<Expression>& entries)
{
	this->mode = EncodingMode::Float;
	this->entries = entries;
	this->writeTermination = false;
}

void CDirectiveData::setDouble(std::vector<Expression>& entries)
{
	this->mode = EncodingMode::Double;
	this->entries = entries;
	this->writeTermination = false;
}

void CDirectiveData::setAscii(std::vector<Expression>& entries, bool terminate)
{
	this->mode = EncodingMode::Ascii;
	this->entries = entries;
	this->writeTermination = terminate;
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
	case EncodingMode::Float:
		return 4;
	case EncodingMode::U64:
	case EncodingMode::Double:
		return 8;
	case EncodingMode::Invalid:
		break;
	}

	return 0;
}

size_t CDirectiveData::getDataSize() const
{
	switch (mode)
	{
	case EncodingMode::Sjis:
	case EncodingMode::Custom:
		return customData.size();
	case EncodingMode::U8:
	case EncodingMode::Ascii:
	case EncodingMode::U16:
	case EncodingMode::U32:
	case EncodingMode::U64:
	case EncodingMode::Float:
	case EncodingMode::Double:
		return normalData.size()*getUnitSize();
	case EncodingMode::Invalid:
		break;
	}

	return 0;
}

void CDirectiveData::encodeCustom(EncodingTable& table)
{
	customData.clear();
	for (size_t i = 0; i < entries.size(); i++)
	{
		ExpressionValue value = entries[i].evaluate();
		if (!value.isValid())
		{
			Logger::queueError(Logger::Error, "Invalid expression");
			continue;
		}
		
		if (value.isInt())
		{
			customData.appendByte((byte)value.intValue);
		} else if (value.isString())
		{
			ByteArray encoded = table.encodeString(value.strValue.string(),false);
			if (encoded.size() == 0 && value.strValue.size() > 0)
			{
				Logger::queueError(Logger::Error, "Failed to encode \"%s\"",value.strValue);
			}
			customData.append(encoded);
		} else {
			Logger::queueError(Logger::Error, "Invalid expression type");
		}
	}

	if (writeTermination)
	{
		ByteArray encoded = table.encodeTermination();
		customData.append(encoded);
	}
}

void CDirectiveData::encodeSjis()
{
	static EncodingTable sjisTable;
	if (!sjisTable.isLoaded())
	{
		unsigned char hexBuffer[2];
		
		sjisTable.setTerminationEntry((unsigned char*)"\0",1);

		for (unsigned short SJISValue = 0x0001; SJISValue < 0x0100; SJISValue++)
		{
			if (auto unicodeValue = sjisToUnicode(SJISValue))
			{
				hexBuffer[0] = SJISValue & 0xFF;
				sjisTable.addEntry(hexBuffer, 1, convertUnicodeCharToUtf8(*unicodeValue));
			}
		}
		for (unsigned short SJISValue = 0x8100; SJISValue < 0xEF00; SJISValue++)
		{
			if (auto unicodeValue = sjisToUnicode(SJISValue))
			{
				hexBuffer[0] = (SJISValue >> 8) & 0xFF;
				hexBuffer[1] = SJISValue & 0xFF;
				sjisTable.addEntry(hexBuffer, 2, convertUnicodeCharToUtf8(*unicodeValue));
			}
		}
	}

	encodeCustom(sjisTable);
}

void CDirectiveData::encodeFloat()
{
	normalData.clear();
	for (size_t i = 0; i < entries.size(); i++)
	{
		ExpressionValue value = entries[i].evaluate();
		if (!value.isValid())
		{
			Logger::queueError(Logger::Error, "Invalid expression");
			continue;
		}

		if (value.isInt() && mode == EncodingMode::Float)
		{
			int32_t num = getFloatBits((float)value.intValue);
			normalData.push_back(num);
		} else if (value.isInt() && mode == EncodingMode::Double)
		{
			int64_t num = getDoubleBits((double)value.intValue);
			normalData.push_back(num);
		} else if (value.isFloat() && mode == EncodingMode::Float)
		{
			int32_t num = getFloatBits((float)value.floatValue);
			normalData.push_back(num);
		} else if (value.isFloat() && mode == EncodingMode::Double)
		{
			int64_t num = getDoubleBits((double)value.floatValue);
			normalData.push_back(num);
		} else {
			Logger::queueError(Logger::Error, "Invalid expression type");
		}
	}
}

void CDirectiveData::encodeNormal()
{
	normalData.clear();
	for (size_t i = 0; i < entries.size(); i++)
	{
		ExpressionValue value = entries[i].evaluate();
		if (!value.isValid())
		{
			Logger::queueError(Logger::Error, "Invalid expression");
			continue;
		}

		if (value.isString())
		{
			bool hadNonAscii = false;
			for (size_t l = 0; l < value.strValue.size(); l++)
			{
				uint64_t num = value.strValue.string()[l];
				normalData.push_back(num);

				if (num >= 0x80 && !hadNonAscii)
				{
					Logger::queueError(Logger::Error, "Non-ASCII character in data directive. Use .string instead");
					hadNonAscii = true;
				}
			}
		} else if (value.isInt())
		{
			int64_t num = value.intValue;
			normalData.push_back(num);
		} else if (value.isFloat() && mode == EncodingMode::U32)
		{
			int32_t num = getFloatBits((float)value.floatValue);
			normalData.push_back(num);
		} else if(value.isFloat() && mode == EncodingMode::U64) {
			int64_t num = getDoubleBits((double)value.floatValue);
			normalData.push_back(num);
		} else {
			Logger::queueError(Logger::Error, "Invalid expression type");
		}
	}

	if (writeTermination)
	{
		normalData.push_back(0);
	}
}

bool CDirectiveData::Validate(const ValidateState &state)
{
	position = g_fileManager->getVirtualAddress();

	size_t oldSize = getDataSize();
	switch (mode)
	{
	case EncodingMode::U8:
	case EncodingMode::U16:
	case EncodingMode::U32:
	case EncodingMode::U64:
	case EncodingMode::Ascii:
		encodeNormal();
		break;
	case EncodingMode::Float:
	case EncodingMode::Double:
		encodeFloat();
		break;
	case EncodingMode::Sjis:
		encodeSjis();
		break;
	case EncodingMode::Custom:
		encodeCustom(Global.Table);
		break;
	default:
		Logger::queueError(Logger::Error, "Invalid encoding type");
		break;
	}

	g_fileManager->advanceMemory(getDataSize());
	return oldSize != getDataSize();
}

void CDirectiveData::Encode() const
{
	switch (mode)
	{
	case EncodingMode::Sjis:
	case EncodingMode::Custom:
		g_fileManager->write(customData.data(),customData.size());
		break;
	case EncodingMode::U8:
	case EncodingMode::Ascii:
		for (auto value: normalData)
		{
			g_fileManager->writeU8((uint8_t)value);
		}
		break;
	case EncodingMode::U16:
		for (auto value: normalData)
		{
			g_fileManager->writeU16((uint16_t)value);
		}
		break;
	case EncodingMode::U32:
	case EncodingMode::Float:
		for (auto value: normalData)
		{
			g_fileManager->writeU32((uint32_t)value);
		}
		break;
	case EncodingMode::U64:
	case EncodingMode::Double:
		for (auto value: normalData)
		{
			g_fileManager->writeU64((uint64_t)value);
		}
		break;
	case EncodingMode::Invalid:
		// TODO: Assert?
		break;
	}
}

void CDirectiveData::writeTempData(TempData& tempData) const
{
	size_t size = (getUnitSize()*2+3)*getDataSize()+20;
	char* str = new char[size];
	char* start = str;

	switch (mode)
	{
	case EncodingMode::Sjis:
	case EncodingMode::Custom:
		str += snprintf(str,20,".byte ");

		for (size_t i = 0; i < customData.size(); i++)
		{
			str += snprintf(str,20,"0x%02X,",(uint8_t)customData[i]);
		}
		break;
	case EncodingMode::U8:
	case EncodingMode::Ascii:
		str += snprintf(str,20,".byte ");
		
		for (size_t i = 0; i < normalData.size(); i++)
		{
			str += snprintf(str,20,"0x%02X,",(uint8_t)normalData[i]);
		}
		break;
	case EncodingMode::U16:
		str += snprintf(str,20,".halfword ");

		for (size_t i = 0; i < normalData.size(); i++)
		{
			str += snprintf(str,20,"0x%04X,",(uint16_t)normalData[i]);
		}
		break;
	case EncodingMode::U32:
	case EncodingMode::Float:
		str += snprintf(str,20, ".word ");

		for (size_t i = 0; i < normalData.size(); i++)
		{
			str += snprintf(str,20,"0x%08X,",(uint32_t)normalData[i]);
		}
		break;
	case EncodingMode::U64:
	case EncodingMode::Double:
		str += snprintf(str,20,".doubleword ");

		for (size_t i = 0; i < normalData.size(); i++)
		{
			str += snprintf(str,20,"0x%16" PRIx64 ",",(uint64_t)normalData[i]);
		}
		break;
	case EncodingMode::Invalid:
		// TODO: Assert?
		str[0] = '\0';
		break;
	}

	tempData.writeLine(position,start);
	delete[] start;
}

void CDirectiveData::writeSymData(SymbolData& symData) const
{
	switch (mode)
	{
	case EncodingMode::Ascii:
		symData.addData(position,getDataSize(),SymbolData::DataAscii);
		break;
	case EncodingMode::U8:
	case EncodingMode::Sjis:
	case EncodingMode::Custom:
		symData.addData(position,getDataSize(),SymbolData::Data8);
		break;
	case EncodingMode::U16:
		symData.addData(position,getDataSize(),SymbolData::Data16);
		break;
	case EncodingMode::U32:
	case EncodingMode::Float:
		symData.addData(position,getDataSize(),SymbolData::Data32);
		break;
	case EncodingMode::U64:
	case EncodingMode::Double:
		symData.addData(position,getDataSize(),SymbolData::Data64);
		break;
	case EncodingMode::Invalid:
		// TODO: Assert?
		break;
	}
}
