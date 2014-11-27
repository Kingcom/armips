#include "stdafx.h"
#include "Expression.h"
#include "Common.h"
#include "ExpressionParser.h"

enum class ExpressionValueCombination { II = 0, IF = 1, FI = 2, FF = 3};

ExpressionValueCombination getValueCombination(ExpressionValueType a, ExpressionValueType b)
{
	u32 b0 = a == ExpressionValueType::Float;
	u32 b1 = b == ExpressionValueType::Float;
	return (ExpressionValueCombination) (b1 | (b0 << 1));
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
		result.intValue = intValue % other.intValue;
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
		result.intValue = intValue << other.intValue;
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
		result.intValue = intValue >> other.intValue;
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
	}

	return false;
}

bool ExpressionValue::operator>(const ExpressionValue& other) const
{
	return !(*this <= other);
}

bool ExpressionValue::operator>=(const ExpressionValue& other) const
{
	return !(*this < other);
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
	}

	return result;
}


ExpressionInternal::ExpressionInternal(u64 value)
{
	type = OperatorType::Integer;
	intValue = value;
}

ExpressionInternal::ExpressionInternal(double value)
{
	type = OperatorType::Float;
	floatValue = value;
}

ExpressionInternal::ExpressionInternal(const std::wstring& value)
{
	type = OperatorType::Identifier;
	label = Global.symbolTable.getLabel(value,Global.FileInfo.FileNum,Global.Section);
}

ExpressionInternal::ExpressionInternal(OperatorType op, ExpressionInternal* a,
	ExpressionInternal* b, ExpressionInternal* c)
{
	type = op;
	children[0] = std::shared_ptr<ExpressionInternal>(a);
	children[1] = std::shared_ptr<ExpressionInternal>(b);
	children[2] = std::shared_ptr<ExpressionInternal>(c);
}


bool ExpressionInternal::hasIdentifierChild()
{
	for (int i = 0; i < 3; i++)
	{
		if (children[i] != NULL)
		{
			if (children[i]->type == OperatorType::Identifier
				|| children[i]->type == OperatorType::MemoryPos
				|| children[i]->hasIdentifierChild())
				return true;
		}
	}

	return false;
}

ExpressionValue ExpressionInternal::evaluate()
{
	ExpressionValue val;

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
		if (!label->isDefined())
		{
			Logger::queueError(Logger::Error,L"Undefined label \"%s\"",label->getName());
			return val;
		}

		val.type = ExpressionValueType::Integer;
		val.intValue = label->getValue();
		return val;
	case OperatorType::MemoryPos:
		val.type = ExpressionValueType::Integer;
		val.intValue = g_fileManager->getVirtualAddress();
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
	default:
		return val;
	}
}


Expression::Expression()
{
	expression = NULL;
}

bool Expression::load(const std::wstring& text, bool allowLabels)
{
	ExpressionParser parser;
	expression = std::shared_ptr<ExpressionInternal>(parser.parse(text));

	if (expression == NULL)
		return false;

	if (!allowLabels && expression->hasIdentifierChild())
		return false;

	originalText = text;
	return true;
}

ExpressionValue Expression::evaluate()
{
	if (expression == NULL)
	{
		ExpressionValue invalid;
		return invalid;
	}

	return expression->evaluate();
}

