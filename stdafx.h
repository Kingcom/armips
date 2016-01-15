
#pragma once

#define _CRT_SECURE_NO_WARNINGS
#undef __STRICT_ANSI__

#ifdef __clang__
typedef struct { double x, y; } __float128;
#endif

#include <cstdio>
#include <vector>
#include <cstdlib>
#include <cstdarg>
#include <cctype>
#include <cstring>
#include <cmath>

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
