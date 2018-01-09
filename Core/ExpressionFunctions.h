#pragma once
#include "Expression.h"
#include <map>

bool getExpFuncParameter(const std::vector<ExpressionValue>& parameters, size_t index, int64_t& dest,
	const std::wstring& funcName, bool optional);

bool getExpFuncParameter(const std::vector<ExpressionValue>& parameters, size_t index, const std::wstring*& dest,
	const std::wstring& funcName, bool optional);

using ExpressionFunction = ExpressionValue (*)(const std::wstring& funcName, const std::vector<ExpressionValue>&);

enum class ExpFuncSafety
{
	// Result may depend entirely on the internal state
	Unsafe,
	// Result is unsafe in conditional blocks, safe otherwise
	ConditionalUnsafe,
	// Result is completely independent of the internal state
	Safe,
};

struct ExpressionFunctionEntry
{
	ExpressionFunction function;
	size_t minParams;
	size_t maxParams;
	ExpFuncSafety safety;
};

using ExpressionFunctionMap =  std::map<std::wstring, const ExpressionFunctionEntry>;;

extern const ExpressionFunctionMap expressionFunctions;

ExpressionValue expFuncDefined(ExpressionInternal* exp);

