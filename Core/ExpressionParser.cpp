#include "stdafx.h"
#include "ExpressionParser.h"
#include "Common.h"
#include "Expression.h"

ExpressionInternal* ExpressionParser::primaryExpression()
{
	Token tok = peekToken();

	if (tok.type == TokenType::Invalid)
		return NULL;

	switch (tok.type)
	{
	case TokenType::Float:
		eatToken();
		return new ExpressionInternal(tok.floatNumber);
	case TokenType::Identifier:
		eatToken();
		return new ExpressionInternal(tok.text,OperatorType::Identifier);
	case TokenType::String:
		eatToken();
		return new ExpressionInternal(tok.text,OperatorType::String);
	case TokenType::Integer:
		eatToken();
		return new ExpressionInternal(tok.intNumber);
	case TokenType::Period:
		eatToken();
		return new ExpressionInternal(OperatorType::MemoryPos);
	case TokenType::LParen:
		eatToken();
		ExpressionInternal* exp = expression();
			
		if (nextToken().type != TokenType::RParen)
			return NULL;

		return exp;
	}

	return NULL;
}

ExpressionInternal* ExpressionParser::unaryExpression()
{
	ExpressionInternal* exp = primaryExpression();
	if (exp != NULL)
		return exp;

	Token op = nextToken();
	exp = primaryExpression();
	if (exp == NULL)
		return NULL;

	switch (op.type)
	{
	case TokenType::Plus:
		return exp;
	case TokenType::Minus:
		return new ExpressionInternal(OperatorType::Neg,exp);
	case TokenType::Tilde:
		return new ExpressionInternal(OperatorType::BitNot,exp);
	case TokenType::Exclamation:
		return new ExpressionInternal(OperatorType::LogNot,exp);
	default:
		return NULL;
	}
}

ExpressionInternal* ExpressionParser::multiplicativeExpression()
{
	ExpressionInternal* exp = unaryExpression();
	if (exp ==  NULL)
		return NULL;

	while (true)
	{
		OperatorType op = OperatorType::Invalid;
		switch (peekToken().type)
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
		}

		if (op == OperatorType::Invalid)
			break;

		eatToken();

		ExpressionInternal* exp2 = unaryExpression();
		if (exp2 == NULL)
			return NULL;

		exp = new ExpressionInternal(op,exp,exp2);
	}

	return exp;
}

ExpressionInternal* ExpressionParser::additiveExpression()
{
	ExpressionInternal* exp = multiplicativeExpression();
	if (exp ==  NULL)
		return NULL;

	while (true)
	{
		OperatorType op = OperatorType::Invalid;
		switch (peekToken().type)
		{
		case TokenType::Plus:
			op = OperatorType::Add;
			break;
		case TokenType::Minus:
			op = OperatorType::Sub;
			break;
		}

		if (op == OperatorType::Invalid)
			break;

		eatToken();

		ExpressionInternal* exp2 = multiplicativeExpression();
		if (exp2 == NULL)
			return NULL;

		exp = new ExpressionInternal(op,exp,exp2);
	}

	return exp;
}

ExpressionInternal* ExpressionParser::shiftExpression()
{
	ExpressionInternal* exp = additiveExpression();
	if (exp ==  NULL)
		return NULL;

	while (true)
	{
		OperatorType op = OperatorType::Invalid;
		switch (peekToken().type)
		{
		case TokenType::LeftShift:
			op = OperatorType::LeftShift;
			break;
		case TokenType::RightShift:
			op = OperatorType::RightShift;
			break;
		}

		if (op == OperatorType::Invalid)
			break;

		eatToken();

		ExpressionInternal* exp2 = additiveExpression();
		if (exp2 == NULL)
			return NULL;

		exp = new ExpressionInternal(op,exp,exp2);
	}

	return exp;
}

ExpressionInternal* ExpressionParser::relationalExpression()
{
	ExpressionInternal* exp = shiftExpression();
	if (exp ==  NULL)
		return NULL;

	while (true)
	{
		OperatorType op = OperatorType::Invalid;
		switch (peekToken().type)
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
		}

		if (op == OperatorType::Invalid)
			break;

		eatToken();

		ExpressionInternal* exp2 = shiftExpression();
		if (exp2 == NULL)
			return NULL;

		exp = new ExpressionInternal(op,exp,exp2);
	}

	return exp;
}

ExpressionInternal* ExpressionParser::equalityExpression()
{
	ExpressionInternal* exp = relationalExpression();
	if (exp ==  NULL)
		return NULL;

	while (true)
	{
		OperatorType op = OperatorType::Invalid;
		switch (peekToken().type)
		{
		case TokenType::Equal:
			op = OperatorType::Equal;
			break;
		case TokenType::NotEqual:
			op = OperatorType::NotEqual;
			break;
		}

		if (op == OperatorType::Invalid)
			break;

		eatToken();

		ExpressionInternal* exp2 = relationalExpression();
		if (exp2 == NULL)
			return NULL;

		exp = new ExpressionInternal(op,exp,exp2);
	}

	return exp;
}

ExpressionInternal* ExpressionParser::andExpression()
{
	ExpressionInternal* exp = equalityExpression();
	if (exp ==  NULL)
		return NULL;

	while (peekToken().type == TokenType::BitAnd)
	{
		eatToken();

		ExpressionInternal* exp2 = equalityExpression();
		if (exp2 == NULL)
			return NULL;

		exp = new ExpressionInternal(OperatorType::BitAnd,exp,exp2);
	}

	return exp;
}

ExpressionInternal* ExpressionParser::exclusiveOrExpression()
{
	ExpressionInternal* exp = andExpression();
	if (exp ==  NULL)
		return NULL;

	while (peekToken().type == TokenType::Caret)
	{
		eatToken();

		ExpressionInternal* exp2 = andExpression();
		if (exp2 == NULL)
			return NULL;

		exp = new ExpressionInternal(OperatorType::Xor,exp,exp2);
	}

	return exp;
}

ExpressionInternal* ExpressionParser::inclusiveOrExpression()
{
	ExpressionInternal* exp = exclusiveOrExpression();
	if (exp ==  NULL)
		return NULL;

	while (peekToken().type == TokenType::BitOr)
	{
		eatToken();

		ExpressionInternal* exp2 = exclusiveOrExpression();
		if (exp2 == NULL)
			return NULL;

		exp = new ExpressionInternal(OperatorType::BitOr,exp,exp2);
	}

	return exp;
}

ExpressionInternal* ExpressionParser::logicalAndExpression()
{
	ExpressionInternal* exp = inclusiveOrExpression();
	if (exp ==  NULL)
		return NULL;

	while (peekToken().type == TokenType::LogAnd)
	{
		eatToken();

		ExpressionInternal* exp2 = inclusiveOrExpression();
		if (exp2 == NULL)
			return NULL;

		exp = new ExpressionInternal(OperatorType::LogAnd,exp,exp2);
	}

	return exp;
}

ExpressionInternal* ExpressionParser::logicalOrExpression()
{
	ExpressionInternal* exp = logicalAndExpression();
	if (exp ==  NULL)
		return NULL;

	while (peekToken().type == TokenType::LogOr)
	{
		eatToken();

		ExpressionInternal* exp2 = logicalAndExpression();
		if (exp2 == NULL)
			return NULL;

		exp = new ExpressionInternal(OperatorType::LogOr,exp,exp2);
	}

	return exp;
}

ExpressionInternal* ExpressionParser::conditionalExpression()
{
	Token tok;

	ExpressionInternal* exp = logicalOrExpression();
	if (exp == NULL)
		return NULL;

	// check a ? b : c
	if (peekToken().type != TokenType::Question)
		return exp;

	eatToken();
	ExpressionInternal* second = expression();

	if (second == NULL)
		return NULL;
	
	if (nextToken().type != TokenType::Colon)
		return NULL;

	ExpressionInternal* third = expression();
	
	if (third == NULL)
		return NULL;

	return new ExpressionInternal(OperatorType::TertiaryIf,exp,second,third);
}

ExpressionInternal* ExpressionParser::expression()
{
	return conditionalExpression();
}

bool ExpressionParser::convertInteger(size_t start, size_t end, u64& result)
{
	// find base of number
	int base = 10;
	if (input[start] == '$')
	{
		base = 16;
		start++;
	} else if (input[start] == '0')
	{
		if (tolower(input[start+1]) == 'x')
		{
			base = 16;
			start += 2;
		} else if (tolower(input[start+1]) == 'o')
		{
			base = 8;
			start += 2;
		}
	}

	if (base == 10)
	{
		if (input[end-1] == 'h')
		{
			base = 16;
			end--;
		} else if (input[end-1] == 'b')
		{
			base = 2;
			end--;
		} else if (input[end-1] == 'o')
		{
			base = 8;
			end--;
		}
	}

	// convert number
	result = 0;
	while (start < end)
	{
		wchar_t c = towlower(input[start++]);

		u32 value = c >= 'a' ? c-'a'+10 : c-'0';

		if ((base == 16 && value > 15) || (base == 10 && value > 9)
			|| (base == 8 && value > 7) || (base == 2 && value > 1))
		{
			return false;
		}

		result = (result*base) + value;
	}

	return true;
}

bool ExpressionParser::convertFloat(size_t start, size_t end, double& result)
{
	std::string str;

	for (size_t i = start; i < end; i++)
	{
		wchar_t c = input[i];
		if (c != '.' && (c < '0' || c > '9'))
			return false;

		str += (char) c;
	}

	result = atof(str.c_str());
	return true;
}

TokenType ExpressionParser::parseOperator()
{
	wchar_t first = input[inputPos];
	wchar_t second = inputPos+1 >= input.size() ? '\0' : input[inputPos+1];

	switch (first)
	{
	case '(':
		inputPos++;
		return TokenType::LParen;
	case ')':
		inputPos++;
		return TokenType::RParen;
	case '+':
		inputPos++;
		return TokenType::Plus;
	case '-':
		inputPos++;
		return TokenType::Minus;
	case '*':
		inputPos++;
		return TokenType::Mult;
	case '/':
		inputPos++;
		return TokenType::Div;
	case '%':
		inputPos++;
		return TokenType::Mod;
	case '^':
		inputPos++;
		return TokenType::Caret;
	case '~':
		inputPos++;
		return TokenType::Tilde;
	case '<':
		if (second == '<')
		{
			inputPos += 2;
			return TokenType::LeftShift;
		}

		if (second == '=')
		{
			inputPos += 2;
			return TokenType::LessEqual;
		}
	
		inputPos++;
		return TokenType::Less;
	case '>':
		if (second == '>')
		{
			inputPos += 2;
			return TokenType::RightShift;
		}

		if (second == '=')
		{
			inputPos += 2;
			return TokenType::GreaterEqual;
		}
		
		inputPos++;
		return TokenType::Greater;
	case '=':
		if (second == '=')
		{
			inputPos += 2;
			return TokenType::Equal;
		}

		return TokenType::Invalid;
	case '!':
		if (second == '=')
		{
			inputPos += 2;
			return TokenType::NotEqual;
		}
		
		inputPos++;
		return TokenType::Exclamation;
	case '&':
		if (second == '&')
		{
			inputPos += 2;
			return TokenType::LogAnd;
		}
		
		inputPos++;
		return TokenType::BitAnd;
	case '|':
		if (second == '|')
		{
			inputPos += 2;
			return TokenType::LogOr;
		}
		
		inputPos++;
		return TokenType::BitOr;
	case '?':
		inputPos++;
		return TokenType::Question;
	case ':':
		inputPos++;
		return TokenType::Colon;
	case '.':
		inputPos++;
		return TokenType::Period;
	default:
		return TokenType::Invalid;
	}
}


bool ExpressionParser::loadToken()
{
	currentToken.text.clear();

	currentToken.type = parseOperator();
	if (currentToken.type != TokenType::Invalid)
		return true;

	// character constants
	wchar_t first = input[inputPos];
	if (first == '\'')
	{
		if (inputPos+3 > input.size() || input[inputPos+2] != '\'')
			return false;

		currentToken.type = TokenType::Integer;
		currentToken.intNumber = input[inputPos+1];
		
		inputPos += 3;
		return true;
	}

	// strings
	if (first == '"')
	{
		inputPos++;

		bool valid = false;
		while (inputPos < input.size())
		{
			if (inputPos+1 < input.size() && input[inputPos] == '\\' && input[inputPos+1] == '"')
			{
				currentToken.text += '"';
				inputPos += 2;
				continue;
			}

			if (input[inputPos] == '"')
			{
				inputPos++;
				valid = true;
				break;
			}

			currentToken.text += input[inputPos++];
		}

		if (!valid)
			return false;
		
		currentToken.type = TokenType::String;
		return true;
	}

	// numbers
	if ((first >= '0' && first <= '9') || first == '$')
	{
		// find end of number
		size_t start = inputPos;
		size_t end = inputPos;
		bool isFloat = false;
		while (inputPos < input.size() && (iswalnum(input[inputPos]) || input[inputPos] == '.'))
		{
			if (input[inputPos] == '.')
			{
				if (isFloat == true)
					return false;
				isFloat = true;
			}

			end++;
			inputPos++;
		}
			
		if (!isFloat)
		{
			currentToken.type = TokenType::Integer;
			if (convertInteger(start,end,currentToken.intNumber) == false)
				return false;
		} else { // isFloat
			currentToken.type = TokenType::Float;
			if (convertFloat(start,end,currentToken.floatNumber) == false)
				return false;
		}
		
		return true;
	}

	// identifiers
	currentToken.type = TokenType::Identifier;
	bool isFirst = true;
	while (inputPos < input.size() && Global.symbolTable.isValidSymbolCharacter(input[inputPos],isFirst))
	{
		currentToken.text += input[inputPos++];
		isFirst = false;
	}

	if (currentToken.text.empty())
	{
		currentToken.type = TokenType::Invalid;
		return false;
	}

	return true;
}

Token& ExpressionParser::nextToken()
{
	if (needNewToken)
		loadToken();

	needNewToken = true;
	return currentToken;
}

Token& ExpressionParser::peekToken()
{
	if (needNewToken)
	{
		loadToken();
		needNewToken = false;
	}
	
	return currentToken;
}

void ExpressionParser::eatToken()
{
	if (needNewToken)
		loadToken();

	needNewToken = true;
}

ExpressionInternal* ExpressionParser::parse(const std::wstring& text)
{
	input = text;
	inputPos = 0;
	needNewToken = true;
	error = false;

	ExpressionInternal* exp = expression();
	if (exp == NULL || error)
		return NULL;

	return exp;
}
