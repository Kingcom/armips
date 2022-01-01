#include "Util/Util.h"

#include <cstring>
#include <sstream>

std::string convertUnicodeCharToUtf8(char32_t character)
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

std::string convertWStringToUtf8(std::wstring_view source)
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

bool stringToInt(const std::string& line, size_t start, size_t end, int64_t& result)
{
	// find base of number
	int32_t base = 10;
	if (line[start] == '0')
	{
		if (tolower(line[start+1]) == 'x')
		{
			base = 16;
			start += 2;
		} else if (tolower(line[start+1]) == 'o')
		{
			base = 8;
			start += 2;
		} else if (tolower(line[start+1]) == 'b' && tolower(line[end-1]) != 'h')
		{
			base = 2;
			start += 2;
		}
	}

	if (base == 10)
	{
		if (tolower(line[end-1]) == 'h')
		{
			base = 16;
			end--;
		} else if (tolower(line[end-1]) == 'b')
		{
			base = 2;
			end--;
		} else if (tolower(line[end-1]) == 'o')
		{
			base = 8;
			end--;
		}
	}

	// convert number
	result = 0;
	while (start < end)
	{
		int c = tolower(line[start++]);

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

std::string toLowercase(const std::string& str)
{
	std::string result;
	for (size_t i = 0; i < str.size(); i++)
	{
		result += tolower(str[i]);
	}

	return result;
}

size_t replaceAll(std::string& str, const char* oldValue,const std::string& newValue)
{
	size_t pos = 0;
	size_t len = strlen(oldValue);

	size_t count = 0;
	while ((pos = str.find(oldValue, pos)) != std::string::npos)
	{
		str.replace(pos,len,newValue);
		pos += newValue.length();
		count++;
	}

	return count;
}

bool startsWith(const std::string& str, const char* value, size_t stringPos)
{
	while (*value != 0 && stringPos < str.size())
	{
		if (str[stringPos++] != *value++)
			return false;
	}

	return *value == 0;
}
