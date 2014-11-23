
#pragma once

#define _CRT_SECURE_NO_WARNINGS
#undef __STRICT_ANSI__

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
#include <windows.h>

typedef unsigned __int64 u64;
typedef unsigned int u32;
#else
typedef uint64_t u64;
typedef uint32_t u32;
#endif
