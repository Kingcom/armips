#pragma once

#include <cstdint>
#include <string>
#include <vector>

std::string convertUnicodeCharToUtf8(char32_t character);
std::string convertWStringToUtf8(std::wstring_view source);

bool stringToInt(const std::string& line, size_t start, size_t end, int64_t& result);
int32_t getFloatBits(float value);
float bitsToFloat(int32_t value);
int64_t getDoubleBits(double value);

std::string toLowercase(const std::string& str);
size_t replaceAll(std::string& str, const char* oldValue,const std::string& newValue);
bool startsWith(const std::string& str, const char* value, size_t stringPos = 0);
