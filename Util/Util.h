#pragma once

#include "Util/FileSystem.h"

#include <string>
#include <vector>

std::wstring convertUtf8ToWString(const char* source);
std::string convertWCharToUtf8(wchar_t character);
std::string convertWStringToUtf8(const std::wstring& source);

std::wstring intToHexString(unsigned int value, int digits, bool prefix = false);
std::wstring intToString(unsigned int value, int digits);
bool stringToInt(const std::wstring& line, size_t start, size_t end, int64_t& result);
int32_t getFloatBits(float value);
float bitsToFloat(int32_t value);
int64_t getDoubleBits(double value);

std::vector<std::wstring> getStringListFromArray(wchar_t** source, int count);
std::vector<std::wstring> splitString(const std::wstring& str, const wchar_t delim, bool skipEmpty);

std::wstring toWLowercase(const std::string& str);
size_t replaceAll(std::wstring& str, const wchar_t* oldValue,const std::wstring& newValue);
bool startsWith(const std::wstring& str, const wchar_t* value, size_t stringPos = 0);
