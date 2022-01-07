#pragma once

#include <optional>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

class ExpressionFunctionHandle;
class ExpressionInternal;
class Identifier;
class Label;

struct ExpressionValue;
struct Token;

using ExpressionFunction = ExpressionValue (*)(const Identifier& funcName, const std::vector<ExpressionValue>&);
using ExpressionLabelFunction = ExpressionValue (*)(const Identifier& funcName, const std::vector<std::shared_ptr<Label>> &);

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

	std::optional<ExpressionFunctionHandle> find(const Identifier &name) const;

	void reset();
	void updateArchitecture();

	bool addFunction(const Identifier &name, ExpressionFunction functor, size_t minParams, size_t maxParams, ExpFuncSafety safety);
	bool addLabelFunction(const Identifier &name, ExpressionLabelFunction functor, size_t minParams, size_t maxParams, ExpFuncSafety safety);
	bool addUserFunction(const Identifier &name, const std::vector<Identifier> &parameters, const std::vector<Token> &content);
private:
	struct Entry
	{
		std::function<ExpressionValue(const std::vector<std::unique_ptr<ExpressionInternal>> &)> f;
		size_t minParams = 0;
		size_t maxParams = 0;
		ExpFuncSafety safety = ExpFuncSafety::Unsafe;
	};

	ExpressionFunctionHandler();
	bool registerEntry(const Identifier &name, Entry entry);

	std::map<Identifier,Entry> entries;
	std::vector<Identifier> architectureFunctions;
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
