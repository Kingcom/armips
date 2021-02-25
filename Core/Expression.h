#pragma once

#include <memory>
#include <string>
#include <vector>

class Label;

struct ExpressionFunctionEntry;
struct ExpressionLabelFunctionEntry;

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
	ToString,
	FunctionCall
};

enum class ExpressionValueType { Invalid, Integer, Float, String};

struct ExpressionValue
{
	ExpressionValueType type;

	ExpressionValue()
	{
		type = ExpressionValueType::Invalid;
		intValue = 0;
	}

	ExpressionValue(int64_t value)
	{
		type = ExpressionValueType::Integer;
		intValue = value;
	}

	ExpressionValue(double value)
	{
		type = ExpressionValueType::Float;
		floatValue = value;
	}

	ExpressionValue(const std::wstring& value)
	{
		type = ExpressionValueType::String;
		strValue = value;
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

	union
	{
		int64_t intValue;
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

class ExpressionInternal
{
public:
	ExpressionInternal();
	~ExpressionInternal();
	ExpressionInternal(int64_t value);
	ExpressionInternal(double value);
	ExpressionInternal(const std::wstring& value, OperatorType type);
	ExpressionInternal(OperatorType op, ExpressionInternal* a = nullptr,
		ExpressionInternal* b = nullptr, ExpressionInternal* c = nullptr);
	ExpressionInternal(const std::wstring& name, const std::vector<ExpressionInternal*>& parameters);
	ExpressionValue evaluate();
	std::wstring toString();
	bool isIdentifier() { return type == OperatorType::Identifier; }
	std::wstring getStringValue() { return strValue; }
	void replaceMemoryPos(const std::wstring& identifierName);
	bool simplify(bool inUnknownOrFalseBlock);
	unsigned int getFileNum() { return fileNum; }
	unsigned int getSection() { return section; }
private:
	void allocate(size_t count);
	void deallocate();
	std::wstring formatFunctionCall();
	ExpressionValue executeExpressionFunctionCall(const ExpressionFunctionEntry& entry);
	ExpressionValue executeExpressionLabelFunctionCall(const ExpressionLabelFunctionEntry& entry);
	ExpressionValue executeFunctionCall();
	bool checkParameterCount(size_t min, size_t max);

	OperatorType type;
	ExpressionInternal** children;
	size_t childrenCount;

	union
	{
		int64_t intValue;
		double floatValue;
	};
	std::wstring strValue;

	unsigned int fileNum, section;
};

class Expression
{
public:
	Expression();
	ExpressionValue evaluate();
	bool isLoaded() const { return expression != nullptr; }
	void setExpression(ExpressionInternal* exp, bool inUnknownOrFalseBlock);
	void replaceMemoryPos(const std::wstring& identifierName);
	bool isConstExpression() { return constExpression; }

	template<typename T>
	bool evaluateInteger(T& dest)
	{
		if (expression == nullptr)
			return false;

		ExpressionValue value = expression->evaluate();
		if (value.isInt() == false)
			return false;

		dest = (T) value.intValue;
		return true;
	}

	bool evaluateString(std::wstring& dest, bool convert);
	bool evaluateIdentifier(std::wstring& dest);
	std::wstring toString();
private:
	std::shared_ptr<ExpressionInternal> expression;
	std::wstring originalText;
	bool constExpression;
};

Expression createConstExpression(int64_t value);
