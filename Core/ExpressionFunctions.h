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

struct ExpressionUserFunction {
	std::wstring name;
	std::vector<std::wstring> parameters;
	std::wstring content;
};

typedef std::map<std::wstring, const ExpressionFunctionEntry> ExpressionFunctionMap;
typedef std::map<std::wstring, ExpressionUserFunction> ExpressionUserFunctionMap;

extern const ExpressionFunctionMap expressionFunctions;

ExpressionValue expFuncDefined(ExpressionInternal* exp);
