#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Core/ExpressionFunctionHandler.h"

bool getExpFuncParameter(const std::vector<ExpressionValue>& parameters, size_t index, int64_t& dest,
	const std::wstring& funcName, bool optional);

bool getExpFuncParameter(const std::vector<ExpressionValue>& parameters, size_t index, const std::wstring*& dest,
	const std::wstring& funcName, bool optional);

struct ExpressionFunctionEntry
{
	const wchar_t *name;
	ExpressionFunction function;
	size_t minParams;
	size_t maxParams;
	ExpFuncSafety safety;
};

struct ExpressionLabelFunctionEntry
{
	const wchar_t *name;
	ExpressionLabelFunction function;
	size_t minParams;
	size_t maxParams;
	ExpFuncSafety safety;
};

void registerExpressionFunctions(ExpressionFunctionHandler &handler);
