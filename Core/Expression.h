#pragma once
#include <memory>

enum class OperatorType
{
	Invalid,
	Integer,
	Float,
	Identifier,
	String,
	MemoryPos,
	Add,
	Sub,
	Mult,
	Div,
	Mod,
	Neg,
	LogNot,
	BitNot,
	LeftShift,
	RightShift,
	Less,
	Greater,
	LessEqual,
	GreaterEqual,
	Equal,
	NotEqual,
	BitAnd,
	Xor,
	BitOr,
	LogAnd,
	LogOr,
	TertiaryIf,
};

enum class ExpressionValueType { Invalid, Integer, Float, String};

struct ExpressionValue
{
	ExpressionValueType type;

	ExpressionValue()
	{
		type = ExpressionValueType::Invalid;
	}

	bool isFloat() const
	{
		return type == ExpressionValueType::Float;
	}
	
	bool isInt() const
	{
		return type == ExpressionValueType::Integer;
	}

	bool isString() const
	{
		return type == ExpressionValueType::String;
	}

	bool isValid() const
	{
		return type != ExpressionValueType::Invalid;
	}

	struct
	{
		u64 intValue;
		double floatValue;
	};

	std::wstring strValue;
	
	ExpressionValue operator!() const;
	ExpressionValue operator~() const;
	bool operator<(const ExpressionValue& other) const;
	bool operator<=(const ExpressionValue& other) const;
	bool operator>(const ExpressionValue& other) const;
	bool operator>=(const ExpressionValue& other) const;
	bool operator==(const ExpressionValue& other) const;
	bool operator!=(const ExpressionValue& other) const;
	ExpressionValue operator+(const ExpressionValue& other) const;
	ExpressionValue operator-(const ExpressionValue& other) const;
	ExpressionValue operator*(const ExpressionValue& other) const;
	ExpressionValue operator/(const ExpressionValue& other) const;
	ExpressionValue operator%(const ExpressionValue& other) const;
	ExpressionValue operator<<(const ExpressionValue& other) const;
	ExpressionValue operator>>(const ExpressionValue& other) const;
	ExpressionValue operator&(const ExpressionValue& other) const;
	ExpressionValue operator|(const ExpressionValue& other) const;
	ExpressionValue operator&&(const ExpressionValue& other) const;
	ExpressionValue operator||(const ExpressionValue& other) const;
	ExpressionValue operator^(const ExpressionValue& other) const;
};

class Label;

class ExpressionInternal
{
public:
	ExpressionInternal(u64 value);
	ExpressionInternal(double value);
	ExpressionInternal(const std::wstring& value, OperatorType type);
	ExpressionInternal(OperatorType op, ExpressionInternal* a = NULL,
		ExpressionInternal* b = NULL, ExpressionInternal* c = NULL);
	ExpressionValue evaluate();
	bool hasIdentifierChild();
private:
	OperatorType type;
	std::shared_ptr<ExpressionInternal> children[3];
	union
	{
		u64 intValue;
		double floatValue;
	};
	std::wstring strValue;
	Label* label;
};

class Expression
{
public:
	Expression();
	bool load(const std::wstring& text, bool allowLabels = true);
	ExpressionValue evaluate();
	bool isLoaded() { return expression != NULL; }

	template<typename T>
	bool evaluateInteger(T& dest)
	{
		if (expression == NULL)
			return false;

		ExpressionValue value = expression->evaluate();
		if (value.isInt() == false)
			return false;

		dest = (T) value.intValue;
		return true;
	}

	bool evaluateString(std::wstring& dest, bool convert)
	{
		if (expression == NULL)
			return false;

		ExpressionValue value = expression->evaluate();
		if (convert && value.isInt())
		{
			dest = std::to_wstring(value.intValue);
			return true;
		}

		if (convert && value.isFloat())
		{
			dest = std::to_wstring(value.floatValue);
			return true;
		}

		if (value.isString() == false)
			return false;

		dest = value.strValue;
		return true;
	}
private:
	std::shared_ptr<ExpressionInternal> expression;
	std::wstring originalText;
};

template<typename T>
bool convertConstExpression(const std::wstring& text, T& dest)
{
	Expression exp;
	if (exp.load(text,false) == false)
		return false;

	ExpressionValue value = exp.evaluate();
	if (value.isInt() == false)
		return false;

	dest = (T) value.intValue;
	return true;
}