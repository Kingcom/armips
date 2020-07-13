#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Parser/Tokenizer.h"

class Label;

struct ExpressionValue;

bool getExpFuncParameter(const std::vector<ExpressionValue>& parameters, size_t index, int64_t& dest,
	const std::wstring& funcName, bool optional);

bool getExpFuncParameter(const std::vector<ExpressionValue>& parameters, size_t index, const std::wstring*& dest,
	const std::wstring& funcName, bool optional);

using ExpressionFunction = ExpressionValue (*)(const std::wstring& funcName, const std::vector<ExpressionValue>&);
using ExpressionLabelFunction = ExpressionValue (*)(const std::wstring& funcName, const std::vector<std::shared_ptr<Label>> &);

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

struct ExpressionLabelFunctionEntry
{
	ExpressionLabelFunction function;
	size_t minParams;
	size_t maxParams;
	ExpFuncSafety safety;
};

struct UserExpressionFunction
{
	std::wstring name;
	std::vector<std::wstring> parameters;
	std::vector<Token> content;
};

class UserFunctions
{
public:
	static UserFunctions &instance()
	{
		static UserFunctions func;
		return func;
	}

	bool addFunction(const UserExpressionFunction &func)
	{
		_entries.emplace(func.name, func);
		return true;
	}
	const UserExpressionFunction *findFunction(const std::wstring &name) const
	{
		auto it = _entries.find(name);
		return it != _entries.end() ? &it->second : nullptr;
	}

private:
	UserFunctions() = default;

	std::map<std::wstring, UserExpressionFunction> _entries;
};

using ExpressionFunctionMap =  std::map<std::wstring, const ExpressionFunctionEntry>;
using ExpressionLabelFunctionMap =  std::map<std::wstring, const ExpressionLabelFunctionEntry>;

extern const ExpressionFunctionMap expressionFunctions;
extern const ExpressionLabelFunctionMap expressionLabelFunctions;
