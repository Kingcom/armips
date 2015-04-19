#pragma once

enum class TokenType
{
	Invalid,
	Identifier,
	Integer,
	String,
	Float,
	LParen,
	RParen,
	Plus,
	Minus,
	Mult,
	Div,
	Mod,
	Caret,
	Tilde,
	LeftShift,
	RightShift,
	Less,
	Greater,
	LessEqual,
	GreaterEqual,
	Equal,
	NotEqual,
	BitAnd,
	BitOr,
	LogAnd,
	LogOr,
	Exclamation,
	Question,
	Colon,
	Period,
};

struct Token
{
	TokenType type;
	std::wstring text;
	u64 intNumber;
	double floatNumber;
};

class ExpressionInternal;

class ExpressionParser
{
public:
	ExpressionInternal* parse(const std::wstring& text);
private:
	bool getToken(size_t index, Token& dest);
	bool convertInteger(size_t start, size_t end, u64& result);
	bool convertFloat(size_t start, size_t end, double& result);
	TokenType parseOperator();

	bool loadToken();
	Token& nextToken();
	Token& peekToken();
	void eatToken();

	ExpressionInternal* primaryExpression();
	ExpressionInternal* unaryExpression();
	ExpressionInternal* multiplicativeExpression();
	ExpressionInternal* additiveExpression();
	ExpressionInternal* shiftExpression();
	ExpressionInternal* relationalExpression();
	ExpressionInternal* equalityExpression();
	ExpressionInternal* andExpression();
	ExpressionInternal* exclusiveOrExpression();
	ExpressionInternal* inclusiveOrExpression();
	ExpressionInternal* logicalAndExpression();
	ExpressionInternal* logicalOrExpression();
	ExpressionInternal* conditionalExpression();
	ExpressionInternal* expression();

	Token currentToken;
	bool needNewToken;
	std::wstring input;
	size_t inputPos;
	bool error;
};