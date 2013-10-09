#pragma once
#include <string>

typedef std::vector<std::wstring> StringList;

std::wstring convertUtf8ToWString(const char* source);
std::string convertWStringToUtf8(const std::wstring& source);
std::wstring formatString(const wchar_t* format, ...);
std::string formatString(const char* format, ...);

class Formatter
{
public:
	Formatter(const std::wstring& string): str(string) { };
	Formatter(const std::string& string) { str = convertUtf8ToWString(string.c_str()); };

	operator std::string() { return convertWStringToUtf8(str); };
	operator std::wstring() { return str; };

	Formatter arg(const std::wstring& s);
	Formatter arg(const std::string& s);
	Formatter arg(unsigned int value, int base = 10, int width = 0, wchar_t filler = 0);
private:
	std::wstring str;
};

std::wstring intToHexString(unsigned int value, int digits, bool prefix = false);
std::wstring intToString(unsigned int value, int digits);

StringList getStringListFromArray(wchar_t** source, int count);