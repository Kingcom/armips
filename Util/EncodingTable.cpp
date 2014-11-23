#include "stdafx.h"
#include "Util/EncodingTable.h"
#include "Util/Util.h"
#include "Core/Common.h"
#include "Core/MathParser.h"

#define MAXHEXLENGTH 32

EncodingTable::EncodingTable()
{

}

EncodingTable::~EncodingTable()
{

}

void EncodingTable::clear()
{
	hexData.clear();
	valueData.clear();
	entries.clear();
}

int parseHexString(std::wstring& hex, unsigned char* dest)
{
	for (size_t i = 0; i < hex.size(); i++)
	{
		wchar_t source = towlower(hex[i]);
		int value;

		if (source >= 'a' && source <= 'f')
		{
			value = source-'a'+10;
		} else if (source >= '0' && source <= '9')
		{
			value = source-'0';
		} else {
			return -1;
		}

		size_t index = i/2;
		if (i % 2)
			dest[index] = (dest[index] << 4) | value;
		else
			dest[index] = value;
	}

	return (int) hex.size()/2;
}

bool EncodingTable::load(const std::wstring& fileName, TextFile::Encoding encoding)
{
	unsigned char hexBuffer[MAXHEXLENGTH];

	TextFile input;
	if (input.open(fileName,TextFile::Read,encoding) == false)
		return false;

	hexData.clear();
	valueData.clear();
	entries.clear();
	setTerminationEntry((unsigned char*)"\0",1);

	while (!input.atEnd())
	{
		std::wstring line = input.readLine();
		if (line.empty() || line[0] == '*') continue;
		
		if (line[0] == '/')
		{
			std::wstring hex = line.substr(1);
			if (hex.empty() || hex.length() > 2*MAXHEXLENGTH)
			{
				// error
				continue;
			}

			int length = parseHexString(hex,hexBuffer);
			if (length == -1)
			{
				// error
				continue;
			}

			setTerminationEntry(hexBuffer,length);
		} else {
			size_t pos = line.find(L'=');
			std::wstring hex = line.substr(0,pos);
			std::wstring value = line.substr(pos+1);

			if (hex.empty() || value.empty() || hex.length() > 2*MAXHEXLENGTH)
			{
				// error
				continue;
			}
			
			int length = parseHexString(hex,hexBuffer);
			if (length == -1)
			{
				// error
				continue;
			}

			addEntry(hexBuffer,length,value);
		}
	}

	return true;
}

void EncodingTable::addEntry(unsigned char* hex, size_t hexLength, const std::wstring& value)
{
	TableEntry entry;
	entry.hexPos = hexData.append(hex,hexLength);
	entry.hexLen = hexLength;
	entry.valuePos = valueData.size();
	valueData.append(value);
	entry.valueLen = value.size();

	entries.push_back(entry);
}

void EncodingTable::addEntry(unsigned char* hex, size_t hexLength, wchar_t value)
{
	TableEntry entry;
	entry.hexPos = hexData.append(hex,hexLength);
	entry.hexLen = hexLength;
	entry.valuePos = valueData.size();
	valueData += value;
	entry.valueLen = 1;

	entries.push_back(entry);
}

void EncodingTable::setTerminationEntry(unsigned char* hex, size_t hexLength)
{
	terminationEntry.hexPos = hexData.append(hex,hexLength);
	terminationEntry.hexLen = hexLength;
	terminationEntry.valueLen = terminationEntry.valuePos = 0;
}

int EncodingTable::searchStringMatch(const std::wstring& str, size_t pos)
{
	size_t longestLength = 0;
	int longestNum = -1;

	for (size_t i = 0; i < entries.size(); i++)
	{
		TableEntry& entry = entries[i];
		if (entry.valueLen < longestLength) continue;

		bool match = true;
		for (size_t k = 0; k < entry.valueLen; k++)
		{
			if (pos+k >= str.size() || valueData[entry.valuePos+k] != str[pos+k])
			{
				match = false;
				break;
			}
		}

		if (match)
		{
			longestLength = entry.valueLen;
			longestNum = (int) i;
		}
	}

	return longestNum;
}

ByteArray EncodingTable::encodeString(const std::wstring& str, bool writeTermination)
{
	ByteArray result;
	size_t pos = 0;

	while (pos < str.size())
	{
		int index = searchStringMatch(str,pos);
		if (index == -1)
		{
			// error
			return ByteArray();
		}

		TableEntry& entry = entries[index];
		for (size_t i = 0; i < entry.hexLen; i++)
		{
			result.appendByte(hexData[entry.hexPos+i]);
		}

		pos += entry.valueLen;
	}

	if (writeTermination)
	{
		TableEntry& entry = terminationEntry;
		for (size_t i = 0; i < entry.hexLen; i++)
		{
			result.appendByte(hexData[entry.hexPos+i]);
		}
	}

	return result;
}

ByteArray EncodingTable::encodeTermination()
{
	ByteArray result;

	TableEntry& entry = terminationEntry;
	for (size_t i = 0; i < entry.hexLen; i++)
	{
		result.appendByte(hexData[entry.hexPos+i]);
	}

	return result;
}
