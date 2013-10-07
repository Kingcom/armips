#pragma once
#include <string>

typedef std::vector<std::wstring> StringList;

std::wstring convertUtf8ToWString(const char* source);
std::string convertWStringToUtf8(const std::wstring& source);
std::wstring formatString(const wchar_t* format, ...);
std::string formatString(const char* format, ...);
