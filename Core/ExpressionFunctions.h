#pragma once
#include "Expression.h"
#include <map>

typedef ExpressionValue (*ExpressionFunction)(const std::vector<ExpressionValue>&);

struct ExpressionFunctionEntry {
	ExpressionFunction function;
	size_t minParams;
	size_t maxParams;
};

typedef std::map<std::wstring, const ExpressionFunctionEntry> ExpressionFunctionMap;

extern const ExpressionFunctionMap expressionFunctions;
