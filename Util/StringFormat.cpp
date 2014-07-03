#include "stdafx.h"
#include "StringFormat.h"

// wchar_t version

void doFormatString(std::wstringstream& stream, const wchar_t* format)
{
	while (*format != 0)
	{
		stream << *format++;
	}
}

bool doFormatSetup(std::wstringstream& stream, const wchar_t*& format)
{
	if (*format == '0')
	{
		stream << std::setfill(L'0');
		format++;
	} else {
		stream << std::setfill(L' ');

		switch (*format)
		{
		case '-':
			stream << std::left;
			format++;
			break;
		}
	}

	int width = 0;
	while (*format >= '0' && *format <= '9')
	{
		width = (width * 10) + (*format++ - '0');
	}

	stream << std::setw(width) << std::nouppercase;

	// specifier
	switch (*format++)
	{
	case 's':	// strings
	case 'S':
	case 'c':	// char
		break;
	case 'd':	// decimal
	case 'i':
	case 'u':
		stream << std::dec;
		break;
	case 'o':	// octal
		stream << std::oct;
		break;
	case 'x':	// lowercas hex
		stream << std::hex;
		break;
	case 'X':	// uppercase hex
		stream << std::hex << std::uppercase;
		break;
	case '%':	// %
		stream << L'%';
		return false;
	}

	return true;
}
