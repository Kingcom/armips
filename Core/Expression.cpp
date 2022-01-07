#include "Core/Expression.h"

#include "Archs/Architecture.h"
#include "Core/Common.h"
#include "Core/ExpressionFunctionHandler.h"
#include "Core/FileManager.h"
#include "Core/Misc.h"
#include "Parser/ExpressionParser.h"
#include "Util/Util.h"

namespace
{
	std::string to_string(int64_t value)
	{
		return tfm::format("%d", value);
	}

	std::string to_string(double value)
	{
		return tfm::format("%#.17g", value);
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
		result.strValue = StringLiteral(to_string(intValue)) + other.strValue;
		break;
	case ExpressionValueCombination::FS:
		result.type = ExpressionValueType::String;
		result.strValue = StringLiteral(to_string(floatValue)) + other.strValue;
		break;
	case ExpressionValueCombination::SI:
		result.type = ExpressionValueType::String;
		result.strValue = strValue + to_string(other.intValue);
		break;
	case ExpressionValueCombination::SF:
		result.type = ExpressionValueType::String;
		result.strValue = strValue + to_string(other.floatValue);
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
			Logger::queueError(Logger::Warning, "Division overflow in expression");
			return result;
		}
		if (other.intValue == 0)
		{
			result.intValue = ~0;
			Logger::queueError(Logger::Warning, "Integer division by zero in expression");
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
			Logger::queueError(Logger::Warning, "Division overflow in expression");
			return result;
		}
		if (other.intValue == 0)
		{
			result.intValue = intValue;
			Logger::queueError(Logger::Warning, "Integer division by zero in expression");
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
		return StringLiteral(to_string(intValue)) == other.strValue;
	case ExpressionValueCombination::FS:
		return StringLiteral(to_string(floatValue)) == other.strValue;
	case ExpressionValueCombination::SI:
		return strValue == to_string(other.intValue);
	case ExpressionValueCombination::SF:
		return strValue == to_string(other.floatValue);
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
	this->value = value;
}

ExpressionInternal::ExpressionInternal(double value)
	: ExpressionInternal()
{
	type = OperatorType::Float;
	this->value = value;
}

ExpressionInternal::ExpressionInternal(Identifier value)
	: ExpressionInternal()
{
	this->type = OperatorType::Identifier;
	this->value = std::move(value);
	fileNum = Global.FileInfo.FileNum;
	section = Global.Section;
}

ExpressionInternal::ExpressionInternal(StringLiteral value)
	: ExpressionInternal()
{
	this->type = OperatorType::String;
	this->value = std::move(value);
}

ExpressionInternal::ExpressionInternal(const Identifier& name, std::vector<std::unique_ptr<ExpressionInternal>> parameters)
	: ExpressionInternal()
{
	type = OperatorType::FunctionCall;
	this->value = name;
	children = std::move(parameters);
}

void ExpressionInternal::replaceMemoryPos(const Identifier& identifierName)
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
		value = identifierName;
		fileNum = Global.FileInfo.FileNum;
		section = Global.Section;
	}
}

ExpressionValue ExpressionInternal::executeFunctionCall()
{
	const auto &functionName = valueAs<Identifier>();

	auto handle = ExpressionFunctionHandler::instance().find(functionName);
	if (!handle)
	{
		Logger::queueError(Logger::Error, "Unknown function \"%s\"", functionName);
		return {};
	}

	if (handle->minParams() > children.size())
	{
		Logger::queueError(Logger::Error, "Not enough parameters for \"%s\" (%d<%d)", functionName, children.size(), handle->minParams());
		return {};
	}

	if (handle->maxParams() < children.size())
	{
		Logger::queueError(Logger::Error, "Too many parameters for \"%s\" (%d>%d)", functionName, children.size(), handle->maxParams());
		return {};
	}

	return handle->execute(children);
}

bool isExpressionFunctionSafe(const Identifier& name, bool inUnknownOrFalseBlock)
{
	auto handle = ExpressionFunctionHandler::instance().find(name);
	if (!handle)
	{
		// well, a function that doesn't exist at least won't change its behavior...
		return true;
	}

	ExpFuncSafety safety = handle->safety();

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
		if (!isExpressionFunctionSafe(valueAs<Identifier>(), inUnknownOrFalseBlock))
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
			this->value = value.intValue;
			break;
		case ExpressionValueType::Float:
			type = OperatorType::Float;
			this->value = value.floatValue;
			break;
		case ExpressionValueType::String:
			type = OperatorType::String;
			this->value = value.strValue;
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
		val.intValue = valueAs<int64_t>();
		return val;
	case OperatorType::Float:
		val.type = ExpressionValueType::Float;
		val.floatValue = valueAs<double>();
		return val;
	case OperatorType::Identifier:
		label = Global.symbolTable.getLabel(valueAs<Identifier>(),fileNum,section);
		if (label == nullptr)
		{
			Logger::queueError(Logger::Error, "Invalid label name \"%s\"", valueAs<Identifier>());
			return val;
		}

		if (!label->isDefined())
		{
			Logger::queueError(Logger::Error, "Undefined label \"%s\"",label->getName());
			return val;
		}

		val.type = ExpressionValueType::Integer;
		val.intValue = label->getValue();
		return val;
	case OperatorType::String:
		val.type = ExpressionValueType::String;
		val.strValue = valueAs<StringLiteral>();
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

static std::string escapeString(const std::string& text)
{
	std::string result = text;
	replaceAll(result,R"(\)",R"(\\)");
	replaceAll(result,R"(")",R"(\")");

	return tfm::format(R"("%s")",text);
}

std::string ExpressionInternal::formatFunctionCall()
{
	std::string text = valueAs<StringLiteral>().string() + "(";

	for (size_t i = 0; i < children.size(); i++)
	{
		if (i != 0)
			text += ",";
		text += children[i]->toString();
	}

	return text + ")";
}

std::string ExpressionInternal::toString()
{
	switch (type)
	{
	case OperatorType::Integer:
		return tfm::format("%d",valueAs<int64_t>());
	case OperatorType::Float:
		return tfm::format("%g",valueAs<double>());
	case OperatorType::Identifier:
		return valueAs<Identifier>().string();
	case OperatorType::String:
		return escapeString(valueAs<StringLiteral>().string());
	case OperatorType::MemoryPos:
		return ".";
	case OperatorType::Add:
		return tfm::format("(%s + %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Sub:
		return tfm::format("(%s - %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Mult:
		return tfm::format("(%s * %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Div:
		return tfm::format("(%s / %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Mod:
		return tfm::format("(%s %% %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Neg:
		return tfm::format("(-%s)",children[0]->toString());
	case OperatorType::LogNot:
		return tfm::format("(!%s)",children[0]->toString());
	case OperatorType::BitNot:
		return tfm::format("(~%s)",children[0]->toString());
	case OperatorType::LeftShift:
		return tfm::format("(%s << %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::RightShift:
		return tfm::format("(%s >> %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Less:
		return tfm::format("(%s < %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Greater:
		return tfm::format("(%s > %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::LessEqual:
		return tfm::format("(%s <= %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::GreaterEqual:
		return tfm::format("(%s >= %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Equal:
		return tfm::format("(%s == %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::NotEqual:
		return tfm::format("(%s != %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::BitAnd:
		return tfm::format("(%s & %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::BitOr:
		return tfm::format("(%s | %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::LogAnd:
		return tfm::format("(%s && %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::LogOr:
		return tfm::format("(%s || %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Xor:
		return tfm::format("(%s ^ %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::TertiaryIf:
		return tfm::format("(%s ? %s : %s)",children[0]->toString(),children[1]->toString(),children[2]->toString());
	case OperatorType::ToString:
		return tfm::format("(%s%s)",convertUnicodeCharToUtf8(u'\u00B0'),children[0]->toString());
	case OperatorType::FunctionCall:
		return formatFunctionCall();
	default:
		return "";
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

void Expression::replaceMemoryPos(const Identifier& identifierName)
{
	if (expression != nullptr)
		expression->replaceMemoryPos(identifierName);
}

bool Expression::evaluateString(StringLiteral &dest, bool convert)
{
	if (expression == nullptr)
		return false;

	ExpressionValue value = expression->evaluate();
	if (convert && value.isInt())
	{
		dest = to_string(value.intValue);
		return true;
	}

	if (convert && value.isFloat())
	{
		dest = to_string(value.floatValue);
		return true;
	}

	if (!value.isString())
		return false;

	dest = value.strValue;
	return true;
}

bool Expression::evaluateIdentifier(Identifier &dest)
{
	if (expression == nullptr || !expression->isIdentifier())
		return false;

	dest = expression->getIdentifier();
	return true;
}

std::string Expression::toString()
{
	return expression != nullptr ? expression->toString() : "";
}

Expression createConstExpression(int64_t value)
{
	return Expression(std::make_unique<ExpressionInternal>(value), false);
}
