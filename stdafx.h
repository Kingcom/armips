
#pragma once

#define _CRT_SECURE_NO_WARNINGS
#undef __STRICT_ANSI__

#if defined(__clang__)
#if __has_feature(cxx_exceptions)
#define ARMIPS_EXCEPTIONS 1
#else
#define ARMIPS_EXCEPTIONS 0
#endif
#elif defined(_MSC_VER) && defined(_CPPUNWIND)
#define ARMIPS_EXCEPTIONS 1
#elif defined(__EXCEPTIONS) || defined(__cpp_exceptions)
#define ARMIPS_EXCEPTIONS 1
#else
#define ARMIPS_EXCEPTIONS 0
#endif

#include <cstdio>
#include <vector>
#include <cstdlib>
#include <cstdarg>
#include <cctype>
#include <cstring>
#include <cmath>
#include <clocale>

#include <sstream>
#include <iomanip>

#ifdef _WIN32
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned __int64 u64;
#else
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#endif

#include "ext/tinyformat/tinyformat.h"
#define formatString tfm::format
