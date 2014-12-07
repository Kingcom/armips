#pragma once

#include <sstream>

void doFormatString(std::wstringstream& stream, const wchar_t* format);
bool doFormatSetup(std::wstringstream& stream, const wchar_t*& format);

template <typename First, typename... Rest>
void doFormatString(std::wstringstream& stream, const wchar_t* format, First& first, const Rest&... rest)
{
	while (*format != 0)
	{
		if (*format == '%')
		{
			format++;
			if (doFormatSetup(stream,format))
				stream << first;
			doFormatString(stream,format,rest...);
			return;
		}

		stream << *format++;
	}
}

template <typename... Args>
std::wstring formatString(const wchar_t* format, const Args&... args)
{
	std::wstringstream stream;
	doFormatString(stream,format,args...);
	return stream.str();
}
