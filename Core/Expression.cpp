#include "Core/Expression.h"

#include "Archs/Architecture.h"
#include "Core/Common.h"
#include "Core/ExpressionFunctions.h"
#include "Core/FileManager.h"
#include "Core/Misc.h"
#include "Parser/ExpressionParser.h"
#include "Util/Util.h"

namespace
{
	std::wstring to_wstring(int64_t value)
	{
		return tfm::format(L"%d", value);
	}

	std::wstring to_wstring(double value)
	{
		return tfm::format(L"%#.17g", value);
	}
}

enum class ExpressionValueCombination
{
	II = (int(ExpressionValueType::Integer) << 2) | (int(ExpressionValueType::Integer) << 0),
	IF = (int(ExpressionValueType::Integer) << 2) | (int(ExpressionValueType::Float)   << 0),
	FI = (int(ExpressionValueType::Float)   << 2) | (int(ExpressionValueType::Integer) << 0),
	FF = (int(ExpressionValueType::Float)   << 2) | (int(ExpressionValueType::Float)   << 0),
	IS = (int(ExpressionValueType::Integer) << 2) | (int(ExpressionValueType::String)  << 0),
	FS = (int(ExpressionValueType::Float)   << 2) | (int(ExpressionValueType::String)  << 0),
	SI = (int(ExpressionValueType::String)  << 2) | (int(ExpressionValueType::Integer) << 0),
	SF = (int(ExpressionValueType::String)  << 2) | (int(ExpressionValueType::Float)   << 0),
	SS = (int(ExpressionValueType::String)  << 2) | (int(ExpressionValueType::String)  << 0),
};

ExpressionValueCombination getValueCombination(ExpressionValueType a, ExpressionValueType b)
{
	return (ExpressionValueCombination) ((int(a) << 2) | (int(b) << 0));
}

ExpressionValue ExpressionValue::operator+(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		result.intValue = intValue + other.intValue;
		break;
	case ExpressionValueCombination::FI:
		result.type = ExpressionValueType::Float;
		result.floatValue = floatValue + other.intValue;
		break;
	case ExpressionValueCombination::IF:
		result.type = ExpressionValueType::Float;
		result.floatValue = intValue + other.floatValue;
		break;
	case ExpressionValueCombination::FF:
		result.type = ExpressionValueType::Float;
		result.floatValue = floatValue + other.floatValue;
		break;
	case ExpressionValueCombination::IS:
		result.type = ExpressionValueType::String;
		result.strValue = to_wstring(intValue) + other.strValue;
		break;
	case ExpressionValueCombination::FS:
		result.type = ExpressionValueType::String;
		result.strValue = to_wstring(floatValue) + other.strValue;
		break;
	case ExpressionValueCombination::SI:
		result.type = ExpressionValueType::String;
		result.strValue = strValue + to_wstring(other.intValue);
		break;
	case ExpressionValueCombination::SF:
		result.type = ExpressionValueType::String;
		result.strValue = strValue + to_wstring(other.floatValue);
		break;
	case ExpressionValueCombination::SS:
		result.type = ExpressionValueType::String;
		result.strValue = strValue + other.strValue;
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator-(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		result.intValue = intValue - other.intValue;
		break;
	case ExpressionValueCombination::FI:
		result.type = ExpressionValueType::Float;
		result.floatValue = floatValue - other.intValue;
		break;
	case ExpressionValueCombination::IF:
		result.type = ExpressionValueType::Float;
		result.floatValue = intValue - other.floatValue;
		break;
	case ExpressionValueCombination::FF:
		result.type = ExpressionValueType::Float;
		result.floatValue = floatValue - other.floatValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator*(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		result.intValue = intValue * other.intValue;
		break;
	case ExpressionValueCombination::FI:
		result.type = ExpressionValueType::Float;
		result.floatValue = floatValue * other.intValue;
		break;
	case ExpressionValueCombination::IF:
		result.type = ExpressionValueType::Float;
		result.floatValue = intValue * other.floatValue;
		break;
	case ExpressionValueCombination::FF:
		result.type = ExpressionValueType::Float;
		result.floatValue = floatValue * other.floatValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator/(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		if (intValue == INT64_MIN && other.intValue == -1){
			result.intValue = INT64_MIN;
			Logger::queueError(Logger::Warning,L"Division overflow in expression");
			return result;
		}
		if (other.intValue == 0)
		{
			result.intValue = ~0;
			Logger::queueError(Logger::Warning,L"Integer division by zero in expression");
			return result;
		}
		result.intValue = intValue / other.intValue;
		break;
	case ExpressionValueCombination::FI:
		result.type = ExpressionValueType::Float;
		result.floatValue = floatValue / other.intValue;
		break;
	case ExpressionValueCombination::IF:
		result.type = ExpressionValueType::Float;
		result.floatValue = intValue / other.floatValue;
		break;
	case ExpressionValueCombination::FF:
		result.type = ExpressionValueType::Float;
		result.floatValue = floatValue / other.floatValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator%(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		if (intValue == INT64_MIN && other.intValue == -1){
			result.intValue = 0;
			Logger::queueError(Logger::Warning,L"Division overflow in expression");
			return result;
		}
		if (other.intValue == 0)
		{
			result.intValue = intValue;
			Logger::queueError(Logger::Warning,L"Integer division by zero in expression");
			return result;
		}
		result.intValue = intValue % other.intValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator!() const
{
	ExpressionValue result;
	result.type = ExpressionValueType::Integer;
	
	if (isFloat())
		result.intValue = !floatValue;
	else
		result.intValue = !intValue;

	return result;
}

ExpressionValue ExpressionValue::operator~() const
{
	ExpressionValue result;

	if (isInt())
	{
		result.type = ExpressionValueType::Integer;
		result.intValue = ~intValue;
	}

	return result;
}

ExpressionValue ExpressionValue::operator<<(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		result.intValue = ((uint64_t) intValue) << other.intValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator>>(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		result.intValue = ((uint64_t) intValue) >> other.intValue;
		break;
	default:
		break;
	}

	return result;
}

bool ExpressionValue::operator<(const ExpressionValue& other) const
{
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		return intValue < other.intValue;
	case ExpressionValueCombination::FI:
		return floatValue < other.intValue;
	case ExpressionValueCombination::IF:
		return intValue < other.floatValue;
	case ExpressionValueCombination::FF:
		return floatValue < other.floatValue;
	case ExpressionValueCombination::SS:
		return strValue < other.strValue;
	default:
		break;
	}

	return false;
}

bool ExpressionValue::operator<=(const ExpressionValue& other) const
{
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		return intValue <= other.intValue;
	case ExpressionValueCombination::FI:
		return floatValue <= other.intValue;
	case ExpressionValueCombination::IF:
		return intValue <= other.floatValue;
	case ExpressionValueCombination::FF:
		return floatValue <= other.floatValue;
	case ExpressionValueCombination::SS:
		return strValue <= other.strValue;
	default:
		break;
	}

	return false;
}

bool ExpressionValue::operator>(const ExpressionValue& other) const
{
	return other < *this;
}

bool ExpressionValue::operator>=(const ExpressionValue& other) const
{
	return other <= *this;
}

bool ExpressionValue::operator==(const ExpressionValue& other) const
{
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		return intValue == other.intValue;
	case ExpressionValueCombination::FI:
		return floatValue == other.intValue;
	case ExpressionValueCombination::IF:
		return intValue == other.floatValue;
	case ExpressionValueCombination::FF:
		return floatValue == other.floatValue;
	case ExpressionValueCombination::IS:
		return to_wstring(intValue) == other.strValue;
	case ExpressionValueCombination::FS:
		return to_wstring(floatValue) == other.strValue;
	case ExpressionValueCombination::SI:
		return strValue == to_wstring(other.intValue);
	case ExpressionValueCombination::SF:
		return strValue == to_wstring(other.floatValue);
	case ExpressionValueCombination::SS:
		return strValue == other.strValue;
	}

	return false;
}

bool ExpressionValue::operator!=(const ExpressionValue& other) const
{
	return !(*this == other);
}

ExpressionValue ExpressionValue::operator&(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		result.intValue = intValue & other.intValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator|(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		result.intValue = intValue | other.intValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator^(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		result.intValue = intValue ^ other.intValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator&&(const ExpressionValue& other) const
{
	ExpressionValue result;
	result.type = ExpressionValueType::Integer;

	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.intValue = intValue && other.intValue;
		break;
	case ExpressionValueCombination::FI:
		result.floatValue = floatValue && other.intValue;
		break;
	case ExpressionValueCombination::IF:
		result.floatValue = intValue && other.floatValue;
		break;
	case ExpressionValueCombination::FF:
		result.floatValue = floatValue && other.floatValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator||(const ExpressionValue& other) const
{
	ExpressionValue result;
	result.type = ExpressionValueType::Integer;

	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.intValue = intValue || other.intValue;
		break;
	case ExpressionValueCombination::FI:
		result.floatValue = floatValue || other.intValue;
		break;
	case ExpressionValueCombination::IF:
		result.floatValue = intValue || other.floatValue;
		break;
	case ExpressionValueCombination::FF:
		result.floatValue = floatValue || other.floatValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionInternal::ExpressionInternal(int64_t value)
	: ExpressionInternal()
{
	type = OperatorType::Integer;
	intValue = value;
}

ExpressionInternal::ExpressionInternal(double value)
	: ExpressionInternal()
{
	type = OperatorType::Float;
	floatValue = value;
}

ExpressionInternal::ExpressionInternal(const std::wstring& value, OperatorType type)
	: ExpressionInternal()
{
	this->type = type;
	strValue = value;

	switch (type)
	{
	case OperatorType::Identifier:
		fileNum = Global.FileInfo.FileNum;
		section = Global.Section;
		break;
	case OperatorType::String:
		break;
	default:
		break;
	}
}

ExpressionInternal::ExpressionInternal(const std::wstring& name, std::vector<std::unique_ptr<ExpressionInternal>> parameters)
	: ExpressionInternal()
{
	type = OperatorType::FunctionCall;
	strValue = name;
	children = std::move(parameters);
}

void ExpressionInternal::replaceMemoryPos(const std::wstring& identifierName)
{
	for (size_t i = 0; i < children.size(); i++)
	{
		if (children[i])
		{
			children[i]->replaceMemoryPos(identifierName);
		}
	}

	if (type == OperatorType::MemoryPos)
	{
		type = OperatorType::Identifier;
		strValue = identifierName;
		fileNum = Global.FileInfo.FileNum;
		section = Global.Section;
	}
}

bool ExpressionInternal::checkParameterCount(size_t minParams, size_t maxParams)
{
	if (minParams > children.size())
	{
		Logger::queueError(Logger::Error,L"Not enough parameters for \"%s\" (min %d)",strValue,minParams);
		return false;
	}

	if (maxParams < children.size())
	{
		Logger::queueError(Logger::Error,L"Too many parameters for \"%s\" (min %d)",strValue,maxParams);
		return false;
	}

	return true;
}

ExpressionValue ExpressionInternal::executeExpressionFunctionCall(const ExpressionFunctionEntry& entry)
{
	// check parameters
	if (!checkParameterCount(entry.minParams, entry.maxParams))
		return {};

	// evaluate parameters
	std::vector<ExpressionValue> params;
	params.reserve(children.size());

	for (size_t i = 0; i < children.size(); i++)
	{
		ExpressionValue result = children[i]->evaluate();
		if (!result.isValid())
		{
			Logger::queueError(Logger::Error,L"%s: Invalid parameter %d", strValue, i+1);
			return result;
		}

		params.push_back(result);
	}

	// execute
	return entry.function(strValue, params);
}

ExpressionValue ExpressionInternal::executeExpressionLabelFunctionCall(const ExpressionLabelFunctionEntry& entry)
{
	// check parameters
	if (!checkParameterCount(entry.minParams, entry.maxParams))
		return {};

	// evaluate parameters
	std::vector<std::shared_ptr<Label>> params;
	params.reserve(children.size());

	for (size_t i = 0; i < children.size(); i++)
	{
		ExpressionInternal *exp = children[i].get();
		if (!exp || !exp->isIdentifier())
		{
			Logger::queueError(Logger::Error,L"%s: Invalid parameter %d, expecting identifier", strValue, i+1);
			return {};
		}

		const std::wstring& name = exp->getStringValue();
		std::shared_ptr<Label> label = Global.symbolTable.getLabel(name,exp->getFileNum(),exp->getSection());
		params.push_back(label);
	}

	// execute
	return entry.function(strValue, params);
}

ExpressionValue ExpressionInternal::executeFunctionCall()
{
	// try expression functions
	auto expFuncIt = expressionFunctions.find(strValue);
	if (expFuncIt != expressionFunctions.end())
		return executeExpressionFunctionCall(expFuncIt->second);

	// try expression label functions
	auto expLabelFuncIt = expressionLabelFunctions.find(strValue);
	if (expLabelFuncIt != expressionLabelFunctions.end())
		return executeExpressionLabelFunctionCall(expLabelFuncIt->second);

	// try architecture specific expression functions
	auto& archExpressionFunctions = Arch->getExpressionFunctions();
	expFuncIt = archExpressionFunctions.find(strValue);
	if (expFuncIt != archExpressionFunctions.end())
		return executeExpressionFunctionCall(expFuncIt->second);

	// try user defined expression functions
	auto *userFunc = UserFunctions::instance().findFunction(strValue);
	if (userFunc)
	{
		if (!checkParameterCount(userFunc->parameters.size(), userFunc->parameters.size()))
			return {};

		// evaluate parameters
		std::vector<ExpressionValue> params;
		params.reserve(children.size());

		for (size_t i = 0; i < children.size(); i++)
		{
			ExpressionValue result = children[i]->evaluate();
			if (!result.isValid())
			{
				Logger::queueError(Logger::Error,L"%s: Invalid parameter %d", strValue, i+1);
				return result;
			}

			params.push_back(result);
		}

		// instantiate
		TokenStreamTokenizer tok;
		tok.init(userFunc->content);

		for (size_t i = 0; i < children.size(); ++i)
		{
			const auto &paramName = userFunc->parameters[i];
			const auto &paramValue = params[i];

			switch (paramValue.type)
			{
			case ExpressionValueType::Float:
				tok.registerReplacementFloat(paramName, paramValue.floatValue);
				break;
			case ExpressionValueType::String:
				tok.registerReplacementString(paramName, paramValue.strValue);
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
			Logger::queueError(Logger::Error,L"%s: Failed to parse user function expression", strValue);
			return {};
		}

		if (!tok.atEnd())
		{
			Logger::queueError(Logger::Error,L"%s: Unconsumed tokens after parsing user function expresion", strValue);
			return {};
		}

		// evaluate expression
		return result.evaluate();
	}

	// error
	Logger::queueError(Logger::Error, L"Unknown function \"%s\"", strValue);
	return {};
}

bool isExpressionFunctionSafe(const std::wstring& name, bool inUnknownOrFalseBlock)
{
	// expression functions may be unsafe, others are safe
	ExpFuncSafety safety = ExpFuncSafety::Unsafe;
	bool found = false;

	auto it = expressionFunctions.find(name);
	if (it != expressionFunctions.end())
	{
		safety = it->second.safety;
		found = true;
	}

	if (!found)
	{
		auto labelIt = expressionLabelFunctions.find(name);
		if (labelIt != expressionLabelFunctions.end())
		{
			safety = labelIt->second.safety;
			found = true;
		}
	}

	if (!found)
	{
		auto& archExpressionFunctions = Arch->getExpressionFunctions();
		it = archExpressionFunctions.find(name);
		if (it != archExpressionFunctions.end())
		{
			safety = it->second.safety;
			found = true;
		}
	}

	if (inUnknownOrFalseBlock && safety == ExpFuncSafety::ConditionalUnsafe)
		return false;

	return safety != ExpFuncSafety::Unsafe;
}

bool ExpressionInternal::simplify(bool inUnknownOrFalseBlock)
{
	// check if this expression can actually be simplified
	// without causing side effects
	switch (type)
	{
	case OperatorType::Identifier:
	case OperatorType::MemoryPos:
	case OperatorType::ToString:
		return false;
	case OperatorType::FunctionCall:
		if (!isExpressionFunctionSafe(strValue, inUnknownOrFalseBlock))
			return false;
		break;
	default:
		break;
	}

	// check if the same applies to all children
	bool canSimplify = true;
	for (size_t i = 0; i < children.size(); i++)
	{
		if (children[i] != nullptr && !children[i]->simplify(inUnknownOrFalseBlock))
			canSimplify = false;
	}

	// if so, this expression can be evaluated into a constant
	if (canSimplify)
	{
		ExpressionValue value = evaluate();

		switch (value.type)
		{
		case ExpressionValueType::Integer:
			type = OperatorType::Integer;
			intValue = value.intValue;
			break;
		case ExpressionValueType::Float:
			type = OperatorType::Float;
			floatValue = value.floatValue;
			break;
		case ExpressionValueType::String:
			type = OperatorType::String;
			strValue = value.strValue;
			break;
		default:
			type = OperatorType::Invalid;
			break;
		}

		children.clear();
	}

	return canSimplify;
}

ExpressionValue ExpressionInternal::evaluate()
{
	ExpressionValue val;

	std::shared_ptr<Label> label;
	switch (type)
	{
	case OperatorType::Integer:
		val.type = ExpressionValueType::Integer;
		val.intValue = intValue;
		return val;
	case OperatorType::Float:
		val.type = ExpressionValueType::Float;
		val.floatValue = floatValue;
		return val;
	case OperatorType::Identifier:
		label = Global.symbolTable.getLabel(strValue,fileNum,section);
		if (label == nullptr)
		{
			Logger::queueError(Logger::Error,L"Invalid label name \"%s\"",strValue);
			return val;
		}

		if (!label->isDefined())
		{
			Logger::queueError(Logger::Error,L"Undefined label \"%s\"",label->getName());
			return val;
		}

		val.type = ExpressionValueType::Integer;
		val.intValue = label->getValue();
		return val;
	case OperatorType::String:
		val.type = ExpressionValueType::String;
		val.strValue = strValue;
		return val;
	case OperatorType::MemoryPos:
		val.type = ExpressionValueType::Integer;
		val.intValue = g_fileManager->getVirtualAddress();
		return val;
	case OperatorType::ToString:
		val.type = ExpressionValueType::String;
		val.strValue = children[0]->toString();
		return val;
	case OperatorType::Add:
		return children[0]->evaluate() + children[1]->evaluate();
	case OperatorType::Sub:
		return children[0]->evaluate() - children[1]->evaluate();
	case OperatorType::Mult:
		return children[0]->evaluate() * children[1]->evaluate();
	case OperatorType::Div:
		return children[0]->evaluate() / children[1]->evaluate();
	case OperatorType::Mod:
		return children[0]->evaluate() % children[1]->evaluate();
	case OperatorType::Neg:
		val.type = ExpressionValueType::Integer;
		val.intValue = 0;
		return val - children[0]->evaluate();
	case OperatorType::LogNot:
		return !children[0]->evaluate();
	case OperatorType::BitNot:
		return ~children[0]->evaluate();
	case OperatorType::LeftShift:
		return children[0]->evaluate() << children[1]->evaluate();
	case OperatorType::RightShift:
		return children[0]->evaluate() >> children[1]->evaluate();
	case OperatorType::Less:
		val.type = ExpressionValueType::Integer;
		val.intValue = children[0]->evaluate() < children[1]->evaluate();
		return val;
	case OperatorType::Greater:
		val.type = ExpressionValueType::Integer;
		val.intValue = children[0]->evaluate() > children[1]->evaluate();
		return val;
	case OperatorType::LessEqual:
		val.type = ExpressionValueType::Integer;
		val.intValue = children[0]->evaluate() <= children[1]->evaluate();
		return val;
	case OperatorType::GreaterEqual:
		val.type = ExpressionValueType::Integer;
		val.intValue = children[0]->evaluate() >= children[1]->evaluate();
		return val;
	case OperatorType::Equal:
		val.type = ExpressionValueType::Integer;
		val.intValue = children[0]->evaluate() == children[1]->evaluate();
		return val;
	case OperatorType::NotEqual:
		val.type = ExpressionValueType::Integer;
		val.intValue = children[0]->evaluate() != children[1]->evaluate();
		return val;
	case OperatorType::BitAnd:
		return children[0]->evaluate() & children[1]->evaluate();
	case OperatorType::BitOr:
		return children[0]->evaluate() | children[1]->evaluate();
	case OperatorType::LogAnd:
		return children[0]->evaluate() && children[1]->evaluate();
	case OperatorType::LogOr:
		return children[0]->evaluate() || children[1]->evaluate();
	case OperatorType::Xor:
		return children[0]->evaluate() ^ children[1]->evaluate();
	case OperatorType::TertiaryIf:
		val.type = ExpressionValueType::Integer;
		val.intValue = 0;
		if (children[0]->evaluate() == val)
			return children[2]->evaluate();
		else
			return children[1]->evaluate();
	case OperatorType::FunctionCall:
		return executeFunctionCall();
	default:
		return val;
	}
}

static std::wstring escapeString(const std::wstring& text)
{
	std::wstring result = text;
	replaceAll(result,LR"(\)",LR"(\\)");
	replaceAll(result,LR"(")",LR"(\")");

	return tfm::format(LR"("%s")",text);
}

std::wstring ExpressionInternal::formatFunctionCall()
{
	std::wstring text = strValue + L"(";

	for (size_t i = 0; i < children.size(); i++)
	{
		if (i != 0)
			text += L",";
		text += children[i]->toString();
	}

	return text + L")";
}

std::wstring ExpressionInternal::toString()
{
	switch (type)
	{
	case OperatorType::Integer:
		return tfm::format(L"%d",intValue);
	case OperatorType::Float:
		return tfm::format(L"%g",floatValue);
	case OperatorType::Identifier:
		return strValue;
	case OperatorType::String:
		return escapeString(strValue);
	case OperatorType::MemoryPos:
		return L".";
	case OperatorType::Add:
		return tfm::format(L"(%s + %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Sub:
		return tfm::format(L"(%s - %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Mult:
		return tfm::format(L"(%s * %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Div:
		return tfm::format(L"(%s / %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Mod:
		return tfm::format(L"(%s %% %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Neg:
		return tfm::format(L"(-%s)",children[0]->toString());
	case OperatorType::LogNot:
		return tfm::format(L"(!%s)",children[0]->toString());
	case OperatorType::BitNot:
		return tfm::format(L"(~%s)",children[0]->toString());
	case OperatorType::LeftShift:
		return tfm::format(L"(%s << %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::RightShift:
		return tfm::format(L"(%s >> %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Less:
		return tfm::format(L"(%s < %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Greater:
		return tfm::format(L"(%s > %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::LessEqual:
		return tfm::format(L"(%s <= %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::GreaterEqual:
		return tfm::format(L"(%s >= %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Equal:
		return tfm::format(L"(%s == %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::NotEqual:
		return tfm::format(L"(%s != %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::BitAnd:
		return tfm::format(L"(%s & %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::BitOr:
		return tfm::format(L"(%s | %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::LogAnd:
		return tfm::format(L"(%s && %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::LogOr:
		return tfm::format(L"(%s || %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Xor:
		return tfm::format(L"(%s ^ %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::TertiaryIf:
		return tfm::format(L"(%s ? %s : %s)",children[0]->toString(),children[1]->toString(),children[2]->toString());
	case OperatorType::ToString:
		return tfm::format(L"(%c%s)",L'\U000000B0',children[0]->toString());
	case OperatorType::FunctionCall:
		return formatFunctionCall();
	default:
		return L"";
	}
}

Expression::Expression(std::unique_ptr<ExpressionInternal> exp, bool inUnknownOrFalseBlock) :
	expression(std::move(exp))
{
	if (expression)
		constExpression = expression->simplify(inUnknownOrFalseBlock);
}

ExpressionValue Expression::evaluate()
{
	if (expression == nullptr)
	{
		ExpressionValue invalid;
		return invalid;
	}

	return expression->evaluate();
}

void Expression::replaceMemoryPos(const std::wstring& identifierName)
{
	if (expression != nullptr)
		expression->replaceMemoryPos(identifierName);
}

bool Expression::evaluateString(std::wstring &dest, bool convert)
{
	if (expression == nullptr)
		return false;

	ExpressionValue value = expression->evaluate();
	if (convert && value.isInt())
	{
		dest = to_wstring(value.intValue);
		return true;
	}

	if (convert && value.isFloat())
	{
		dest = to_wstring(value.floatValue);
		return true;
	}

	if (!value.isString())
		return false;

	dest = value.strValue;
	return true;
}

bool Expression::evaluateIdentifier(std::wstring &dest)
{
	if (expression == nullptr || !expression->isIdentifier())
		return false;

	dest = expression->getStringValue();
	return true;
}

std::wstring Expression::toString()
{
	return expression != nullptr ? expression->toString() : L"";
}

Expression createConstExpression(int64_t value)
{
	return Expression(std::make_unique<ExpressionInternal>(value), false);
}
