#include "stdafx.h"
#include "Expression.h"
#include "Common.h"

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
		result.strValue = std::to_wstring(intValue) + other.strValue;
		break;
	case ExpressionValueCombination::FS:
		result.type = ExpressionValueType::String;
		result.strValue = std::to_wstring(floatValue) + other.strValue;
		break;
	case ExpressionValueCombination::SI:
		result.type = ExpressionValueType::String;
		result.strValue = strValue + std::to_wstring(other.intValue);
		break;
	case ExpressionValueCombination::SF:
		result.type = ExpressionValueType::String;
		result.strValue = strValue + std::to_wstring(other.floatValue);
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
		if (other.intValue == 0)
		{
			result.type = ExpressionValueType::String;
			result.strValue = L"undef";
			return result;
		}
		result.type = ExpressionValueType::Integer;
		result.intValue = intValue / other.intValue;
		break;
	case ExpressionValueCombination::FI:
		if (other.intValue == 0)
		{
			result.type = ExpressionValueType::String;
			result.strValue = L"undef";
			return result;
		}
		result.type = ExpressionValueType::Float;
		result.floatValue = floatValue / other.intValue;
		break;
	case ExpressionValueCombination::IF:
		if (other.floatValue == 0)
		{
			result.type = ExpressionValueType::String;
			result.strValue = L"undef";
			return result;
		}
		result.type = ExpressionValueType::Float;
		result.floatValue = intValue / other.floatValue;
		break;
	case ExpressionValueCombination::FF:
		if (other.floatValue == 0)
		{
			result.type = ExpressionValueType::String;
			result.strValue = L"undef";
			return result;
		}
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
		if (other.intValue == 0)
		{
			result.type = ExpressionValueType::String;
			result.strValue = L"undef";
			return result;
		}
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
	case ExpressionValueCombination::IS:
		return std::to_wstring(intValue) == other.strValue;
	case ExpressionValueCombination::FS:
		return std::to_wstring(floatValue) == other.strValue;
	case ExpressionValueCombination::SI:
		return strValue == std::to_wstring(other.intValue);
	case ExpressionValueCombination::SF:
		return strValue == std::to_wstring(other.floatValue);
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

ExpressionInternal::ExpressionInternal()
{
	for (size_t i = 0; i < ARRAY_SIZE(children); i++)
	{
		children[i] = nullptr;
	}
}

ExpressionInternal::ExpressionInternal(u64 value)
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
	}
}

ExpressionInternal::ExpressionInternal(OperatorType op, ExpressionInternal* a,
	ExpressionInternal* b, ExpressionInternal* c)
	: ExpressionInternal()
{
	type = op;
	children[0] = a;
	children[1] = b;
	children[2] = c;
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

void ExpressionInternal::replaceMemoryPos(const std::wstring& identifierName)
{
	for (int i = 0; i < 3; i++)
	{
		if (children[i] != NULL)
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

ExpressionValue ExpressionInternal::evaluate()
{
	ExpressionValue val;

	Label* label;
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
	default:
		return val;
	}
}


std::wstring ExpressionInternal::toString()
{
	switch (type)
	{
	case OperatorType::Integer:
		return formatString(L"%d",intValue);
	case OperatorType::Float:
		return formatString(L"%g",floatValue);
	case OperatorType::Identifier:
		return strValue;
	case OperatorType::String:
		return formatString(L"\"%s\"",strValue);
	case OperatorType::MemoryPos:
		return L".";
	case OperatorType::Add:
		return formatString(L"(%s + %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Sub:
		return formatString(L"(%s - %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Mult:
		return formatString(L"(%s * %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Div:
		return formatString(L"(%s / %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Mod:
		return formatString(L"(%s %% %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Neg:
		return formatString(L"(-%s)",children[0]->toString());
	case OperatorType::LogNot:
		return formatString(L"(!%s)",children[0]->toString());
	case OperatorType::BitNot:
		return formatString(L"(~%s)",children[0]->toString());
	case OperatorType::LeftShift:
		return formatString(L"(%s << %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::RightShift:
		return formatString(L"(%s >> %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Less:
		return formatString(L"(%s < %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Greater:
		return formatString(L"(%s > %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::LessEqual:
		return formatString(L"(%s <= %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::GreaterEqual:
		return formatString(L"(%s >= %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Equal:
		return formatString(L"(%s == %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::NotEqual:
		return formatString(L"(%s != %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::BitAnd:
		return formatString(L"(%s & %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::BitOr:
		return formatString(L"(%s | %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::LogAnd:
		return formatString(L"(%s && %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::LogOr:
		return formatString(L"(%s || %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Xor:
		return formatString(L"(%s ^ %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::TertiaryIf:
		return formatString(L"(%s ? %s : %s)",children[0]->toString(),children[1]->toString(),children[2]->toString());
	case OperatorType::ToString:
		return formatString(L"(%c%s)",L'\U000000B0',children[0]->toString());
	default:
		return L"";
	}
}

Expression::Expression()
{
	expression = NULL;
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

void Expression::replaceMemoryPos(const std::wstring& identifierName)
{
	if (expression != NULL)
		expression->replaceMemoryPos(identifierName);
}

Expression createConstExpression(u64 value)
{
	Expression exp;
	ExpressionInternal* num = new ExpressionInternal(value);
	exp.setExpression(num);
	return exp;
}