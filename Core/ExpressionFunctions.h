#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Core/ExpressionFunctionHandler.h"
#include "Core/Types.h"

bool getExpFuncParameter(const std::vector<ExpressionValue>& parameters, size_t index, int64_t& dest,
	const Identifier &funcName, bool optional);

bool getExpFuncParameter(const std::vector<ExpressionValue>& parameters, size_t index, const StringLiteral*& dest,
	const Identifier &funcName, bool optional);

struct ExpressionFunctionEntry
{
	const char *name;
	ExpressionFunction function;
	size_t minParams;
	size_t maxParams;
	ExpFuncSafety safety;
};

struct ExpressionLabelFunctionEntry
{
	const char *name;
	ExpressionLabelFunction function;
	size_t minParams;
	size_t maxParams;
	ExpFuncSafety safety;
};

void registerExpressionFunctions(ExpressionFunctionHandler &handler);
