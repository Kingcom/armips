#include "Util/Util.h"

#include <sstream>

std::wstring convertUtf8ToWString(const char* source)
{
	std::wstring result;

	int index = 0;
	while (source[index] != 0)
	{
		int extraBytes = 0;
		int value = source[index++];
			
		if ((value & 0xE0) == 0xC0)
		{
			extraBytes = 1;
			value &= 0x1F;
		} else if ((value & 0xF0) == 0xE0)
		{
			extraBytes = 2;
			value &= 0x0F;
		} else if (value > 0x7F)
		{
			// error
			return std::wstring();
		}

		for (int i = 0; i < extraBytes; i++)
		{
			int b = source[index++];
			if ((b & 0xC0) != 0x80)
			{
			// error
			return std::wstring();
			}

			value = (value << 6) | (b & 0x3F);
		}

		result += value;
	}

	return result;
}

std::string convertWCharToUtf8(wchar_t character)
{
	std::string result;
	
	if (character < 0x80)
	{
		result += character & 0x7F;
	} else if (character < 0x800)
	{
		result += 0xC0 | ((character >> 6) & 0x1F);
		result += (0x80 | (character & 0x3F));
	} else {
		result += 0xE0 | ((character >> 12) & 0xF);
		result += 0x80 | ((character >> 6) & 0x3F);
		result += 0x80 | (character & 0x3F);
	}

	return result;
}

std::string convertWStringToUtf8(const std::wstring& source)
{
	std::string result;
	
	for (size_t i = 0; i < source.size(); i++)
	{
		wchar_t character = source[i];
		if (character < 0x80)
		{
			result += character & 0x7F;
		} else if (character < 0x800)
		{
			result += 0xC0 | ((character >> 6) & 0x1F);
			result += (0x80 | (character & 0x3F));
		} else {
			result += 0xE0 | ((character >> 12) & 0xF);
			result += 0x80 | ((character >> 6) & 0x3F);
			result += 0x80 | (character & 0x3F);
		}
	}

	return result;
}

std::wstring intToHexString(unsigned int value, int digits, bool prefix)
{
	std::wstring result;
	result.reserve((digits+prefix) ? 2 : 0);

	if (prefix)
	{
		result += '0';
		result += 'x';
	}

	while (digits > 8)
	{
		result += '0';
		digits--;
	}
	
	wchar_t buf[9];
	swprintf(buf,9,L"%0*X",digits,value);
	result += buf;

	return result;
}

std::wstring intToString(unsigned int value, int digits)
{
	std::wstring result;
	result.reserve(digits);

	while (digits > 8)
	{
		result += ' ';
		digits--;
	}
	
	wchar_t buf[9];
	swprintf(buf,9,L"%*d",digits,value);
	result += buf;

	return result;
}

bool stringToInt(const std::wstring& line, size_t start, size_t end, int64_t& result)
{
	// find base of number
	int32_t base = 10;
	if (line[start] == '0')
	{
		if (towlower(line[start+1]) == 'x')
		{
			base = 16;
			start += 2;
		} else if (towlower(line[start+1]) == 'o')
		{
			base = 8;
			start += 2;
		} else if (towlower(line[start+1]) == 'b' && towlower(line[end-1]) != 'h')
		{
			base = 2;
			start += 2;
		}
	}

	if (base == 10)
	{
		if (towlower(line[end-1]) == 'h')
		{
			base = 16;
			end--;
		} else if (towlower(line[end-1]) == 'b')
		{
			base = 2;
			end--;
		} else if (towlower(line[end-1]) == 'o')
		{
			base = 8;
			end--;
		}
	}

	// convert number
	result = 0;
	while (start < end)
	{
		wchar_t c = towlower(line[start++]);

		int32_t value = c >= 'a' ? c-'a'+10 : c-'0';

		if (value >= base)
			return false;

		result = (result*base) + value;
	}

	return true;
}

int32_t getFloatBits(float value)
{
	union { float f; int32_t i; } u;
	u.f = value;
	return u.i;
}

float bitsToFloat(int32_t value)
{
	union { float f; int32_t i; } u;
	u.i = value;
	return u.f;
}

int64_t getDoubleBits(double value)
{
	union { double f; int64_t i; } u;
	u.f = value;
	return u.i;
}

std::vector<std::wstring> getStringListFromArray(wchar_t** source, int count)
{
	std::vector<std::wstring> result;
	for (int i = 0; i < count; i++)
	{
		result.push_back(std::wstring(source[i]));
	}

	return result;
}

std::vector<std::wstring> splitString(const std::wstring& str, const wchar_t delim, bool skipEmpty)
{
	std::vector<std::wstring> result;
	std::wstringstream stream(str);
	std::wstring arg;
	while (std::getline(stream,arg,delim))
	{
		if (arg.empty() && skipEmpty) continue;
		result.push_back(arg);
	}

	return result;
}

std::wstring toWLowercase(const std::string& str)
{
	std::wstring result;
	for (size_t i = 0; i < str.size(); i++)
	{
		result += tolower(str[i]);
	}

	return result;
}

size_t replaceAll(std::wstring& str, const wchar_t* oldValue,const std::wstring& newValue)
{
	size_t pos = 0;
	size_t len = wcslen(oldValue);

	size_t count = 0;
	while ((pos = str.find(oldValue, pos)) != std::string::npos)
	{
		str.replace(pos,len,newValue);
		pos += newValue.length();
		count++;
	}

	return count;
}

bool startsWith(const std::wstring& str, const wchar_t* value, size_t stringPos)
{
	while (*value != 0 && stringPos < str.size())
	{
		if (str[stringPos++] != *value++)
			return false;
	}

	return *value == 0;
}
