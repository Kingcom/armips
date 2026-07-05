#include "Util/Util.h"

#include <cstring>
#include <limits>
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

uint16_t toHalfFloat(double x)
{
	union
	{
		double value;
		struct
		{
			uint64_t mantissa : 52;
			uint64_t exponent : 11;
			uint64_t sign : 1;
		} parts;
	};
	value = x;

	// per https://pspdev.github.io/vfpu-docs/#floating-point-format
	// rounding is hardwired to nearest and subnormals flush to zero
	if (parts.exponent == 0x7FF && parts.mantissa == 0)
	{
		// inf
		return (parts.sign << 15) | (0x1F << 10);
	}
	else if (parts.exponent == 0x7FF && parts.mantissa != 0)
	{
		// NaN
		return (parts.sign << 15) | (0x1F << 10) | 1;
	}
	else if (parts.exponent <= 0x3F0)
	{
		// 0, subnormals, numbers less than 2^-15
		return parts.sign << 15;
	}
	else
	{
		// numbers between 2^-14 (inclusive) and 2^16 (exclusive)
		uint16_t exponent = parts.exponent - 0x3F0;

		// round to nearest (essentially `floor(x + 0.5)`)
		uint64_t mantissa = ((parts.mantissa >> (42 - 1)) + 1) >> 1;
		if (mantissa == 0x400) {
			// round up to the next exponent
			exponent += 1;
			mantissa = 0;
		}
		if (exponent >= 0x1F) {
			// round down to 65504 (greatest finite half float)
			exponent = 0x1E;
			mantissa = 0x3FF;
		}
		return (parts.sign << 15) | (exponent << 10) | mantissa;
	}
}

double fromHalfFloat(uint16_t x)
{
	using nl = std::numeric_limits<double>;
	union
	{
		uint16_t hValue;
		struct
		{
			unsigned int mantissa : 10;
			unsigned int exponent : 5;
			unsigned int sign : 1;
		} hParts;
	};
	hValue = x;

	if (hParts.exponent == 0x1F && hParts.mantissa == 0)
	{
		// inf
		return (hParts.sign ? -1.0 : 1.0) * nl::infinity();
	}
	else if (hParts.exponent == 0x1F && hParts.mantissa != 0)
	{
		// NaN
		return nl::signaling_NaN();
	}
	else if (hParts.exponent == 0)
	{
		// 0
		return hParts.sign ? -0.0 : 0.0;
	}
	else
	{
		// numbers between 2^-14 (inclusive) and 2^16 (exclusive)
		union
		{
			double dValue;
			struct
			{
				uint64_t mantissa : 52;
				uint64_t exponent : 11;
				uint64_t sign : 1;
			} dParts;
		};
		dParts.mantissa = int64_t(hParts.mantissa) << 42;
		dParts.exponent = hParts.exponent + 0x3F0;
		dParts.sign = hParts.sign;
		return dValue;
	}
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
