#include "Parser/ExpressionParser.h"
#include "Core/Expression.h"
#include "Core/ExpressionFunctionHandler.h"
#include "Parser/Tokenizer.h"

static std::unique_ptr<ExpressionInternal> expression(Tokenizer& tokenizer);

static std::unique_ptr<ExpressionInternal> primaryExpression(Tokenizer& tokenizer)
{
	const Token &tok = tokenizer.peekToken();

	switch (tok.type)
	{
	case TokenType::Float:
		tokenizer.eatToken();
		return std::make_unique<ExpressionInternal>(tok.floatValue());
	case TokenType::Identifier:
		{
			const Identifier &identifier = tok.identifierValue();
			tokenizer.eatToken();
			if (identifier == ".")
				return std::make_unique<ExpressionInternal>(OperatorType::MemoryPos);
			else
				return std::make_unique<ExpressionInternal>(identifier);
		}
	case TokenType::String:
		tokenizer.eatToken();
		return std::make_unique<ExpressionInternal>(tok.stringValue());
	case TokenType::Integer:
		tokenizer.eatToken();
		return std::make_unique<ExpressionInternal>(tok.intValue());
	case TokenType::LParen:
		{
			tokenizer.eatToken();
			std::unique_ptr<ExpressionInternal> exp = expression(tokenizer);

			if (tokenizer.nextToken().type != TokenType::RParen)
				return nullptr;

			return exp;
		}
	case TokenType::Invalid:
	default:
		break;
	}

	return nullptr;
}

static std::unique_ptr<ExpressionInternal> postfixExpression(Tokenizer& tokenizer)
{
	if (tokenizer.peekToken(0).type == TokenType::Identifier &&
		tokenizer.peekToken(1).type == TokenType::LParen &&
		ExpressionFunctionHandler::instance().find(tokenizer.peekToken(0).identifierValue()))
	{
		const Identifier &functionName = tokenizer.nextToken().identifierValue();
		tokenizer.eatToken();

		std::vector<std::unique_ptr<ExpressionInternal>> parameters;
		while (tokenizer.peekToken().type != TokenType::RParen)
		{
			if (parameters.size() != 0 && tokenizer.nextToken().type != TokenType::Comma)
				return nullptr;

			std::unique_ptr<ExpressionInternal> exp = expression(tokenizer);
			if (exp == nullptr)
				return nullptr;

			parameters.push_back(std::move(exp));
		}

		tokenizer.eatToken();

		return std::make_unique<ExpressionInternal>(functionName, std::move(parameters));
	}

	return primaryExpression(tokenizer);
}

static std::unique_ptr<ExpressionInternal> unaryExpression(Tokenizer& tokenizer)
{
	std::unique_ptr<ExpressionInternal> exp = postfixExpression(tokenizer);
	if (exp != nullptr)
		return exp;

	const TokenType opType = tokenizer.nextToken().type;
	exp = postfixExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	switch (opType)
	{
	case TokenType::Plus:
		return exp;
	case TokenType::Minus:
		return std::make_unique<ExpressionInternal>(OperatorType::Neg, std::move(exp));
	case TokenType::Tilde:
		return std::make_unique<ExpressionInternal>(OperatorType::BitNot, std::move(exp));
	case TokenType::Exclamation:
		return std::make_unique<ExpressionInternal>(OperatorType::LogNot, std::move(exp));
	case TokenType::Degree:
		return std::make_unique<ExpressionInternal>(OperatorType::ToString, std::move(exp));
	default:
		return nullptr;
	}
}

static std::unique_ptr<ExpressionInternal> multiplicativeExpression(Tokenizer& tokenizer)
{
	std::unique_ptr<ExpressionInternal> exp = unaryExpression(tokenizer);
	if (exp ==  nullptr)
		return nullptr;

	while (true)
	{
		OperatorType op = OperatorType::Invalid;
		switch (tokenizer.peekToken().type)
		{
		case TokenType::Mult:
			op = OperatorType::Mult;
			break;
		case TokenType::Div:
			op = OperatorType::Div;
			break;
		case TokenType::Mod:
			op = OperatorType::Mod;
			break;
		default:
			break;
		}

		if (op == OperatorType::Invalid)
			break;

		tokenizer.eatToken();

		std::unique_ptr<ExpressionInternal> exp2 = unaryExpression(tokenizer);
		if (exp2 == nullptr)
			return nullptr;

		exp = std::make_unique<ExpressionInternal>(op, std::move(exp), std::move(exp2));
	}

	return exp;
}

static std::unique_ptr<ExpressionInternal> additiveExpression(Tokenizer& tokenizer)
{
	std::unique_ptr<ExpressionInternal> exp = multiplicativeExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (true)
	{
		OperatorType op = OperatorType::Invalid;
		switch (tokenizer.peekToken().type)
		{
		case TokenType::Plus:
			op = OperatorType::Add;
			break;
		case TokenType::Minus:
			op = OperatorType::Sub;
			break;
		default:
			break;
		}

		if (op == OperatorType::Invalid)
			break;

		tokenizer.eatToken();

		std::unique_ptr<ExpressionInternal> exp2 = multiplicativeExpression(tokenizer);
		if (exp2 == nullptr)
			return nullptr;

		exp = std::make_unique<ExpressionInternal>(op, std::move(exp), std::move(exp2));
	}

	return exp;
}

static std::unique_ptr<ExpressionInternal> shiftExpression(Tokenizer& tokenizer)
{
	std::unique_ptr<ExpressionInternal> exp = additiveExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (true)
	{
		OperatorType op = OperatorType::Invalid;
		switch (tokenizer.peekToken().type)
		{
		case TokenType::LeftShift:
			op = OperatorType::LeftShift;
			break;
		case TokenType::RightShift:
			op = OperatorType::RightShift;
			break;
		default:
			break;
		}

		if (op == OperatorType::Invalid)
			break;

		tokenizer.eatToken();

		std::unique_ptr<ExpressionInternal> exp2 = additiveExpression(tokenizer);
		if (exp2 == nullptr)
			return nullptr;

		exp = std::make_unique<ExpressionInternal>(op, std::move(exp), std::move(exp2));
	}

	return exp;
}

static std::unique_ptr<ExpressionInternal> relationalExpression(Tokenizer& tokenizer)
{
	std::unique_ptr<ExpressionInternal> exp = shiftExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (true)
	{
		OperatorType op = OperatorType::Invalid;
		switch (tokenizer.peekToken().type)
		{
		case TokenType::Less:
			op = OperatorType::Less;
			break;
		case TokenType::LessEqual:
			op = OperatorType::LessEqual;
			break;
		case TokenType::Greater:
			op = OperatorType::Greater;
			break;
		case TokenType::GreaterEqual:
			op = OperatorType::GreaterEqual;
			break;
		default:
			break;
		}

		if (op == OperatorType::Invalid)
			break;

		tokenizer.eatToken();

		std::unique_ptr<ExpressionInternal> exp2 = shiftExpression(tokenizer);
		if (exp2 == nullptr)
			return nullptr;
		exp = std::make_unique<ExpressionInternal>(op, std::move(exp), std::move(exp2));
	}

	return exp;
}

static std::unique_ptr<ExpressionInternal> equalityExpression(Tokenizer& tokenizer)
{
	std::unique_ptr<ExpressionInternal> exp = relationalExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (true)
	{
		OperatorType op = OperatorType::Invalid;
		switch (tokenizer.peekToken().type)
		{
		case TokenType::Equal:
			op = OperatorType::Equal;
			break;
		case TokenType::NotEqual:
			op = OperatorType::NotEqual;
			break;
		default:
			break;
		}

		if (op == OperatorType::Invalid)
			break;

		tokenizer.eatToken();

		std::unique_ptr<ExpressionInternal> exp2 = relationalExpression(tokenizer);
		if (exp2 == nullptr)
			return nullptr;

		exp = std::make_unique<ExpressionInternal>(op, std::move(exp), std::move(exp2));
	}

	return exp;
}

static std::unique_ptr<ExpressionInternal> andExpression(Tokenizer& tokenizer)
{
	std::unique_ptr<ExpressionInternal> exp = equalityExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (tokenizer.peekToken().type == TokenType::BitAnd)
	{
		tokenizer.eatToken();

		std::unique_ptr<ExpressionInternal> exp2 = equalityExpression(tokenizer);
		if (exp2 == nullptr)
			return nullptr;

		exp = std::make_unique<ExpressionInternal>(OperatorType::BitAnd, std::move(exp), std::move(exp2));
	}

	return exp;
}

static std::unique_ptr<ExpressionInternal> exclusiveOrExpression(Tokenizer& tokenizer)
{
	std::unique_ptr<ExpressionInternal> exp = andExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (tokenizer.peekToken().type == TokenType::Caret)
	{
		tokenizer.eatToken();

		std::unique_ptr<ExpressionInternal> exp2 = andExpression(tokenizer);
		if (exp2 == nullptr)
			return nullptr;

		exp = std::make_unique<ExpressionInternal>(OperatorType::Xor, std::move(exp), std::move(exp2));
	}

	return exp;
}

static std::unique_ptr<ExpressionInternal> inclusiveOrExpression(Tokenizer& tokenizer)
{
	std::unique_ptr<ExpressionInternal> exp = exclusiveOrExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (tokenizer.peekToken().type == TokenType::BitOr)
	{
		tokenizer.eatToken();

		std::unique_ptr<ExpressionInternal> exp2 = exclusiveOrExpression(tokenizer);
		if (exp2 == nullptr)
			return nullptr;

		exp = std::make_unique<ExpressionInternal>(OperatorType::BitOr, std::move(exp), std::move(exp2));
	}

	return exp;
}

static std::unique_ptr<ExpressionInternal> logicalAndExpression(Tokenizer& tokenizer)
{
	std::unique_ptr<ExpressionInternal> exp = inclusiveOrExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (tokenizer.peekToken().type == TokenType::LogAnd)
	{
		tokenizer.eatToken();

		std::unique_ptr<ExpressionInternal> exp2 = inclusiveOrExpression(tokenizer);
		if (exp2 == nullptr)
			return nullptr;

		exp = std::make_unique<ExpressionInternal>(OperatorType::LogAnd, std::move(exp), std::move(exp2));
	}

	return exp;
}

static std::unique_ptr<ExpressionInternal> logicalOrExpression(Tokenizer& tokenizer)
{
	std::unique_ptr<ExpressionInternal> exp = logicalAndExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (tokenizer.peekToken().type == TokenType::LogOr)
	{
		tokenizer.eatToken();

		std::unique_ptr<ExpressionInternal> exp2 = logicalAndExpression(tokenizer);
		if (exp2 == nullptr)
			return nullptr;

		exp = std::make_unique<ExpressionInternal>(OperatorType::LogOr, std::move(exp), std::move(exp2));
	}

	return exp;
}

static std::unique_ptr<ExpressionInternal> conditionalExpression(Tokenizer& tokenizer)
{
	std::unique_ptr<ExpressionInternal> exp = logicalOrExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	// check a ? b : c
	if (tokenizer.peekToken().type != TokenType::Question)
		return exp;

	tokenizer.eatToken();
	std::unique_ptr<ExpressionInternal> second = expression(tokenizer);

	if (second != nullptr && tokenizer.nextToken().type == TokenType::Colon)
	{
		std::unique_ptr<ExpressionInternal> third = expression(tokenizer);
		if (third != nullptr)
			return std::make_unique<ExpressionInternal>(OperatorType::TertiaryIf, std::move(exp), std::move(second), std::move(third));
	}

	return nullptr;
}

static std::unique_ptr<ExpressionInternal> expression(Tokenizer& tokenizer)
{
	return conditionalExpression(tokenizer);
}

Expression parseExpression(Tokenizer& tokenizer, bool inUnknownOrFalseBlock)
{
	TokenizerPosition pos = tokenizer.getPosition();

	// parse expression, revert tokenizer to previous position
	// if it failed
	std::unique_ptr<ExpressionInternal> exp = expression(tokenizer);
	if (exp == nullptr)
		tokenizer.setPosition(pos);

	return Expression(std::move(exp), inUnknownOrFalseBlock);
}
