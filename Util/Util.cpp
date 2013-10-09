#include "stdafx.h"
#include "Util.h"


Formatter Formatter::arg(const std::wstring& s)
{
	// only replace first placeholder for now
	size_t index = str.find(L"%");
	while (index != std::string::npos)
	{
		if (str[index+1] != '%')
		{
			int length = 1;
			while (index+length < str.size() && str[index+length] >= '0' && str[index+length] <= '9')
				length++;

			if (length != 1)
			{
				return str.replace(str.begin()+index,str.begin()+index+length,s);
			}
		}
		index = str.find(L"%",index+1);
	}

	return str;
}

Formatter Formatter::arg(const std::string& s)
{
	return arg(convertUtf8ToWString(s.c_str()));
}

Formatter Formatter::arg(unsigned int value, int base, int width, wchar_t filler)
{
	wchar_t buffer[128];
	int size;

	switch (base)
	{
	case 10:
		size = swprintf(buffer,L"%d",value);
		break;
	case 16:
		size = swprintf(buffer,L"%X",value);
		if (width != 0 && filler == 0) filler = '0'; 
		break;
	default:
		return str;
	}

	std::wstring string;
	if (width != 0 && filler != 0)
	{
		for (int i = 0; i < width-size; i++)
		{
			string.push_back(filler);
		}
	}

	string.append(buffer);
	return arg(string);
}


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

std::wstring intToHexString(unsigned int value, int digits, bool prefix)
{
	std::wstring result;
	result.reserve(digits+prefix ? 2 : 0);

	if (prefix)
	{
		result.push_back('0');
		result.push_back('x');
	}

	while (digits > 8)
	{
		result.push_back('0');
		digits--;
	}
	
	wchar_t buf[9];
	swprintf(buf,L"%0*X",digits,value);
	result += buf;

	return result;
}

std::wstring intToString(unsigned int value, int digits)
{
	std::wstring result;
	result.reserve(digits);

	while (digits > 8)
	{
		result.push_back(' ');
		digits--;
	}
	
	wchar_t buf[9];
	swprintf(buf,L"%*d",digits,value);
	result += buf;

	return result;
}

StringList getStringListFromArray(wchar_t** source, int count)
{
	StringList result;
	for (int i = 0; i < count; i++)
	{
		result.push_back(std::wstring(source[i]));
	}

	return result;
}