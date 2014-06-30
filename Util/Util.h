#pragma once
#include <string>
#include "Util/ByteArray.h"

typedef std::vector<std::wstring> StringList;

std::wstring convertUtf8ToWString(const char* source);
std::string convertWStringToUtf8(const std::wstring& source);
std::wstring formatString(const wchar_t* format, ...);
std::string formatString(const char* format, ...);

class Formatter
{
public:
	Formatter(const std::wstring& string): data(string) { };
	Formatter(const std::string& string) { data = convertUtf8ToWString(string.c_str()); };

	operator std::string() { return convertWStringToUtf8(data); };
	operator std::wstring() { return data; };

	Formatter& arg(const std::wstring& s);
	Formatter& arg(const std::string& s);
	Formatter& arg(unsigned int value, int base = 10, int width = 0, wchar_t filler = 0);
private:
	std::wstring getLowestNumString();

	std::wstring data;
};

std::wstring intToHexString(unsigned int value, int digits, bool prefix = false);
std::wstring intToString(unsigned int value, int digits);

StringList getStringListFromArray(wchar_t** source, int count);

int fileSize(const std::wstring& fileName);
int fileSize(const std::string& fileName);
bool fileExists(const std::wstring& strFilename);
bool fileExists(const std::string& strFilename);

std::wstring toWLowercase(const std::string& str);
std::wstring getFileNameFromPath(const std::wstring& path);

inline unsigned int swapEndianness32(unsigned int value)
{
	return ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) | ((value & 0xFF0000) >> 8) | ((value & 0xFF000000) >> 24);
}

inline unsigned short swapEndianness16(unsigned short value)
{
	return ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);
}