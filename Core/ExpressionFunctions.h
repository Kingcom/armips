#pragma once
#include "Expression.h"
#include <map>

typedef ExpressionValue (*ExpressionFunction)(const std::wstring& funcName, const std::vector<ExpressionValue>&);

struct ExpressionFunctionEntry {
	ExpressionFunction function;
	size_t minParams;
	size_t maxParams;
	bool safe;		// result doesn't depend on the current state
};

typedef std::map<std::wstring, const ExpressionFunctionEntry> ExpressionFunctionMap;

extern const ExpressionFunctionMap expressionFunctions;

ExpressionValue expFuncDefined(ExpressionInternal* exp);
