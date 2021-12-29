#pragma once

#include <optional>
#include <functional>
#include <map>

class ExpressionFunctionHandle;
class ExpressionInternal;
class Label;

struct ExpressionValue;
struct Token;

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

class ExpressionFunctionHandler
{
	friend class ExpressionFunctionHandle;

public:
	static ExpressionFunctionHandler &instance();

	std::optional<ExpressionFunctionHandle> find(const std::wstring &name) const;

	void reset();
	void updateArchitecture();

	bool addFunction(const std::wstring &name, ExpressionFunction functor, size_t minParams, size_t maxParams, ExpFuncSafety safety);
	bool addLabelFunction(const std::wstring &name, ExpressionLabelFunction functor, size_t minParams, size_t maxParams, ExpFuncSafety safety);
	bool addUserFunction(const std::wstring &name, const std::vector<std::wstring> &parameters, const std::vector<Token> &content);
private:
	struct Entry
	{
		std::function<ExpressionValue(const std::vector<std::unique_ptr<ExpressionInternal>> &)> f;
		size_t minParams = 0;
		size_t maxParams = 0;
		ExpFuncSafety safety = ExpFuncSafety::Unsafe;
	};

	ExpressionFunctionHandler();
	bool registerEntry(const std::wstring &name, Entry entry);

	std::map<std::wstring,Entry> entries;
	std::vector<std::wstring> architectureFunctions;
	bool registeringArchitecture = false;
};

class ExpressionFunctionHandle
{
public:
	ExpressionFunctionHandle(const ExpressionFunctionHandler::Entry &entry);

	size_t minParams() const;
	size_t maxParams() const;
	ExpFuncSafety safety() const;
	ExpressionValue execute(const std::vector<std::unique_ptr<ExpressionInternal>> &parameters) const;

private:
	const ExpressionFunctionHandler::Entry &impl;
};
