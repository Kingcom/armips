#include "Core/ExpressionFunctionHandler.h"

#include "Archs/Architecture.h"
#include "Core/Common.h"
#include "Core/Expression.h"
#include "Core/Misc.h"
#include "Parser/ExpressionParser.h"
#include "Parser/Tokenizer.h"

ExpressionFunctionHandler &ExpressionFunctionHandler::instance()
{
	static ExpressionFunctionHandler handler;
	return handler;
}

ExpressionFunctionHandler::ExpressionFunctionHandler()
{
}

std::optional<ExpressionFunctionHandle> ExpressionFunctionHandler::find(const Identifier &name) const
{
	auto it = entries.find(name);
	return it != entries.end() ? std::make_optional(ExpressionFunctionHandle(it->second)) : std::nullopt;
}

void ExpressionFunctionHandler::reset()
{
	entries.clear();
	architectureFunctions.clear();
}

void ExpressionFunctionHandler::updateArchitecture()
{
	// remove functions of previous architecture
	for (const auto &name : architectureFunctions)
	{
		entries.erase(name);
	}

	architectureFunctions.clear();

	// Add functions of current architecture. Remember functions that were added
	registeringArchitecture = true;
	Architecture::current().registerExpressionFunctions(*this);
	registeringArchitecture = false;
}

bool ExpressionFunctionHandler::registerEntry(const Identifier &name, Entry entry)
{
	bool result = entries.emplace(name, std::move(entry)).second;

	if (result && registeringArchitecture)
		architectureFunctions.push_back(name);

	return result;
}

bool ExpressionFunctionHandler::addFunction(const Identifier &name, ExpressionFunction functor, size_t minParams, size_t maxParams, ExpFuncSafety safety)
{
	auto executor = [name, functor](const std::vector<std::unique_ptr<ExpressionInternal>> &parameters) -> ExpressionValue
	{
		// evaluate parameters
		std::vector<ExpressionValue> params;
		params.reserve(parameters.size());

		for (size_t i = 0; i < parameters.size(); i++)
		{
			ExpressionValue result = parameters[i]->evaluate();
			if (!result.isValid())
			{
				Logger::queueError(Logger::Error, "%s: Invalid parameter %d", name, i+1);
				return result;
			}

			params.push_back(result);
		}

		// execute
		return functor(name, params);
	};

	return registerEntry(name, Entry{std::move(executor), minParams, maxParams, safety});
};

bool ExpressionFunctionHandler::addLabelFunction(const Identifier &name, ExpressionLabelFunction functor, size_t minParams, size_t maxParams, ExpFuncSafety safety)
{
	auto executor = [name, functor](const std::vector<std::unique_ptr<ExpressionInternal>> &parameters) -> ExpressionValue
	{
		// evaluate parameters
		std::vector<std::shared_ptr<Label>> params;
		params.reserve(parameters.size());

		for (size_t i = 0; i < parameters.size(); i++)
		{
			ExpressionInternal *exp = parameters[i].get();
			if (!exp || !exp->isIdentifier())
			{
				Logger::queueError(Logger::Error, "%s: Invalid parameter %d, expecting identifier", name, i+1);
				return {};
			}

			const Identifier& name = exp->getIdentifier();
			std::shared_ptr<Label> label = Global.symbolTable.getLabel(name, exp->getFileNum(), exp->getSection());
			params.push_back(label);
		}

		// execute
		return functor(name, params);
	};

	return registerEntry(name, Entry{std::move(executor), minParams, maxParams, safety});
};

bool ExpressionFunctionHandler::addUserFunction(const Identifier &name, const std::vector<Identifier> &parameters, const std::vector<Token> &content)
{
	// Executor: Evaluate parameters and instantiate function content with parameter substitutions
	auto executor = [functionName=name, parameterNames=parameters, content](const std::vector<std::unique_ptr<ExpressionInternal>> &parameters) -> ExpressionValue
	{
		// evaluate parameters
		std::vector<ExpressionValue> params;
		params.reserve(parameters.size());

		for (size_t i = 0; i < parameters.size(); i++)
		{
			ExpressionValue result = parameters[i]->evaluate();
			if (!result.isValid())
			{
				Logger::queueError(Logger::Error, "%s: Invalid parameter %d", functionName, i+1);
				return result;
			}

			params.push_back(result);
		}

		// instantiate
		TokenStreamTokenizer tok;
		tok.init(content);

		for (size_t i = 0; i < parameters.size(); ++i)
		{
			const auto &paramName = parameterNames[i];
			const auto &paramValue = params[i];

			switch (paramValue.type)
			{
			case ExpressionValueType::Float:
				tok.registerReplacementFloat(paramName, paramValue.floatValue);
				break;
			case ExpressionValueType::String:
				tok.registerReplacementString(paramName, paramValue.strValue.string());
				break;
			case ExpressionValueType::Integer:
				tok.registerReplacementInteger(paramName, paramValue.intValue);
				break;
			case ExpressionValueType::Invalid: // will not occur, invalid results are caught above
				break;
			}
		}

		Expression result = parseExpression(tok, false);
		if (!result.isLoaded())
		{
			Logger::queueError(Logger::Error, "%s: Failed to parse user function expression", functionName);
			return {};
		}

		if (!tok.atEnd())
		{
			Logger::queueError(Logger::Error, "%s: Unconsumed tokens after parsing user function expresion", functionName);
			return {};
		}

		// evaluate expression
		return result.evaluate();
	};

	return registerEntry(name, Entry{std::move(executor), parameters.size(), parameters.size(), ExpFuncSafety::Unsafe});
}


ExpressionFunctionHandle::ExpressionFunctionHandle(const ExpressionFunctionHandler::Entry &entry) :
	impl(entry)
{
}

size_t ExpressionFunctionHandle::minParams() const
{
	return impl.minParams;
}

size_t ExpressionFunctionHandle::maxParams() const
{
	return impl.maxParams;
}

ExpFuncSafety ExpressionFunctionHandle::safety() const
{
	return impl.safety;
}

ExpressionValue ExpressionFunctionHandle::execute(const std::vector<std::unique_ptr<ExpressionInternal> > &parameters) const
{
	if (parameters.size() < impl.minParams || parameters.size() > impl.maxParams)
		return {};

	return impl.f(parameters);
}
