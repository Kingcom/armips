#include "stdafx.h"
#include "Util.h"

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

		result.push_back(value);
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
			result.push_back(character & 0x7F);
		} else if (character < 0x800)
		{
			result.push_back(0xC0 | (character >> 6) & 0x1F);
			result.push_back(0x80 | (character & 0x3F));
		} else {
			result.push_back(0xE0 | (character >> 12) & 0xF);
			result.push_back(0x80 | ((character >> 6) & 0x3F));
			result.push_back(0x80 | (character & 0x3F));
		}
	}

	return result;
}

std::wstring formatString(const wchar_t* format, ...)
{
	std::wstring result;
	va_list args;

	va_start(args,format);

	int length = _vscwprintf(format,args);
	if (length < 0) // error
	{
		va_end(args);
		return L"";
	}

	wchar_t* buffer = (wchar_t*) alloca((length+1)*sizeof(wchar_t));
	length = _vsnwprintf(buffer,length+1,format,args);

	if (length >= 0)
		result = buffer;

	va_end(args);
	return result;
}

std::string formatString(const char* format, ...)
{
	std::string result;
	va_list args;

	va_start(args,format);

	int length = _vscprintf(format,args);
	if (length < 0) // error
	{
		va_end(args);
		return "";
	}

	char* buffer = (char*) alloca((length+1)*sizeof(char));
	length = _vsnprintf(buffer,length+1,format,args);

	if (length >= 0)
		result = buffer;

	va_end(args);
	return result;
}