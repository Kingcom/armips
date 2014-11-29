#pragma once
#include "Mips.h"

bool parseVFPURegister(const char* line, MipsVFPURegister& reg, int size);
int parseVFPUCondition(const char* source, int& RetLen);
bool parseVpfxsParameter(const char* text, int& result, int& RetLen);
bool parseVpfxdParameter(const char* text, int& result, int& RetLen);
bool parseVcstParameter(const char* text, int& result, int& RetLen);
bool parseCop2BranchCondition(const char* text, int& result, int& RetLen);
bool parseVfpuControlRegister(const char* text, MipsVFPURegister& reg, int& RetLen);
int parseVfpuVrot(const char* text, int& result, int size, int& RetLen);
int floatToHalfFloat(int i);
