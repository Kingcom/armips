#include "stdafx.h"
#include "Tokenizer.h"
#include "Core/Common.h"
#include <algorithm>


//
// Tokenizer
//

Tokenizer::Tokenizer()
{
	tokenIndex = 0;
	invalidToken.type = TokenType::Invalid;
}

Token& Tokenizer::nextToken()
{
	readTokens(tokenIndex);
	
	if (tokenIndex >= tokens.size())
		return invalidToken;

	return tokens[tokenIndex++];
}

Token& Tokenizer::peekToken(int ahead)
{
	readTokens(tokenIndex+ahead);

	if (tokenIndex+ahead >= tokens.size())
		return invalidToken;

	return tokens[tokenIndex+ahead];
}

void Tokenizer::eatTokens(int num)
{
	tokenIndex += num;
}

void Tokenizer::skipLookahead()
{
	tokenIndex = tokens.size();
}

std::vector<Token> Tokenizer::getTokens(size_t start, size_t count)
{
	std::vector<Token> result;

	size_t oldPos = getPosition();
	setPosition(start);

	for (size_t i = 0; i < count; i++)
	{
		result.push_back(nextToken());
	}

	setPosition(oldPos);
	return result;
}

void Tokenizer::registerReplacement(const std::wstring& identifier, std::vector<Token>& tokens)
{
	Replacement replacement { identifier, tokens };
	replacements.push_back(replacement);
}

void Tokenizer::registerReplacement(const std::wstring& identifier, const std::wstring& newValue)
{
	Token tok;
	tok.type = TokenType::Identifier;
	tok.stringValue = newValue;

	Replacement replacement;
	replacement.identifier = identifier;
	replacement.value.push_back(tok);

	replacements.push_back(replacement);
}

void Tokenizer::readTokens(size_t maxIndex)
{
	while (maxIndex >= tokens.size())
	{
		if (isInputAtEnd())
			return;

		Token token = loadToken();

		// check replacements
		bool replaced = false;
		if (token.type == TokenType::Identifier)
		{
			for (Replacement& replacement: replacements)
			{
				// if the identifier matches, add all of its tokens
				if (replacement.identifier == token.stringValue)
				{
					for (size_t i = 0; i < replacement.value.size(); i++)
					{
						tokens.push_back(replacement.value[i]);
					}

					replaced = true;
					break;
				}
			}
		}

		if (replaced == false)
			tokens.push_back(token);
	}
}


//
// FileTokenizer
//

inline bool isWhitespace(const std::wstring& text, size_t pos)
{
	if (pos >= text.size())
		return false;

	return text[pos] == ' ' || text[pos] == '\t';
}

inline bool isComment(const std::wstring& text, size_t pos)
{
	if (pos < text.size() && text[pos] == ';')
		return true;

	if (pos+1 < text.size() && text[pos+0] == '/' && text[pos+1] == '/')
		return true;

	return false;
}

void FileTokenizer::skipWhitespace()
{
	while (linePos == currentLine.size() || isWhitespace(currentLine,linePos) || isComment(currentLine,linePos))
	{
		// skip whitespace
		while (isWhitespace(currentLine,linePos))
		{
			linePos++;
		}

		// skip comments
		if (isComment(currentLine,linePos))
			linePos = currentLine.size();

		if (linePos >= currentLine.size())
		{
			if (input->atEnd())
				return;

			currentLine = input->readLine();
			linePos = 0;
			lineNumber++;
		}
	}
}

void FileTokenizer::createToken(TokenType type, size_t length)
{
	token.type = type;
	token.line = lineNumber;
	token.column = linePos+1;
	token.originalText = currentLine.substr(linePos,length);

	linePos += length;

	// advance to start of next token
	skipWhitespace();
}

void FileTokenizer::createToken(TokenType type, size_t length, u64 value)
{
	token.type = type;
	token.line = lineNumber;
	token.column = linePos+1;
	token.originalText = currentLine.substr(linePos,length);
	token.intValue = value;

	linePos += length;
	
	// advance to start of next token
	skipWhitespace();
}

void FileTokenizer::createToken(TokenType type, size_t length, double value)
{
	token.type = type;
	token.line = lineNumber;
	token.column = linePos+1;
	token.originalText = currentLine.substr(linePos,length);
	token.floatValue = value;

	linePos += length;
	
	// advance to start of next token
	skipWhitespace();
}

void FileTokenizer::createToken(TokenType type, size_t length, const std::wstring& value)
{
	token.type = type;
	token.line = lineNumber;
	token.column = linePos+1;
	token.originalText = currentLine.substr(linePos,length);
	token.stringValue = value;

	linePos += length;
	
	// advance to start of next token
	skipWhitespace();
}

bool FileTokenizer::parseOperator()
{
	wchar_t first = currentLine[linePos];
	wchar_t second = linePos+1 >= currentLine.size() ? '\0' : currentLine[linePos+1];

	switch (first)
	{
	case '(':
		createToken(TokenType::LParen,1);
		return true;
	case ')':
		createToken(TokenType::RParen,1);
		return true;
	case '+':
		createToken(TokenType::Plus,1);
		return true;
	case '-':
		createToken(TokenType::Minus,1);
		return true;
	case '*':
		createToken(TokenType::Mult,1);
		return true;
	case '/':
		createToken(TokenType::Div,1);
		return true;
	case '%':
		createToken(TokenType::Mod,1);
		return true;
	case '^':
		createToken(TokenType::Caret,1);
		return true;
	case '~':
		createToken(TokenType::Tilde,1);
		return true;
	case '<':
		if (second == '<')
			createToken(TokenType::LeftShift,2);
		else if (second == '=')
			createToken(TokenType::LessEqual,2);
		else
			createToken(TokenType::Less,1);
		return true;
	case '>':
		if (second == '>')
			createToken(TokenType::RightShift,2);
		else if (second == '=')
			createToken(TokenType::GreaterEqual,2);
		else
			createToken(TokenType::Greater,1);
		return true;
	case '=':
		if (second == '=')
			createToken(TokenType::Equal,2);
		else
			createToken(TokenType::Assign,1);
		return true;
	case '!':
		if (second == '=')
			createToken(TokenType::NotEqual,2);
		else
			createToken(TokenType::Exclamation,1);
		return true;
	case '&':
		if (second == '&')
			createToken(TokenType::LogAnd,2);
		else
			createToken(TokenType::BitAnd,1);
		return true;
	case '|':
		if (second == '|')
			createToken(TokenType::LogOr,2);
		else
			createToken(TokenType::BitOr,1);
		return true;
	case '?':
		createToken(TokenType::Question,1);
		return true;
	case ':':
		createToken(TokenType::Colon,1);
		return true;
	case ',':
		createToken(TokenType::Comma,1);
		return true;
	case '[':
		createToken(TokenType::LBrack,1);
		return true;
	case ']':
		createToken(TokenType::RBrack,1);
		return true;
	case '#':
		createToken(TokenType::Hash,1);
		return true;
	case '{':
		createToken(TokenType::LBrace,1);
		return true;
	case '}':
		createToken(TokenType::RBrace,1);
		return true;
	case '$':
		createToken(TokenType::Dollar,1);
		return true;
	}

	return false;
}

bool FileTokenizer::convertInteger(size_t start, size_t end, u64& result)
{
	// find base of number
	int base = 10;
	if (currentLine[start] == '0')
	{
		if (tolower(currentLine[start+1]) == 'x')
		{
			base = 16;
			start += 2;
		} else if (tolower(currentLine[start+1]) == 'o')
		{
			base = 8;
			start += 2;
		}
	}

	if (base == 10)
	{
		if (currentLine[end-1] == 'h')
		{
			base = 16;
			end--;
		} else if (currentLine[end-1] == 'b')
		{
			base = 2;
			end--;
		} else if (currentLine[end-1] == 'o')
		{
			base = 8;
			end--;
		}
	}

	// convert number
	result = 0;
	while (start < end)
	{
		wchar_t c = towlower(currentLine[start++]);

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

bool FileTokenizer::convertFloat(size_t start, size_t end, double& result)
{
	std::string str;

	for (size_t i = start; i < end; i++)
	{
		wchar_t c = currentLine[i];
		if (c != '.' && (c < '0' || c > '9'))
			return false;

		str += (char) c;
	}

	result = atof(str.c_str());
	return true;
}

Token FileTokenizer::loadToken()
{
	if (isInputAtEnd())
	{
		createToken(TokenType::Invalid,0);
		return token;
	}

	size_t pos = linePos;

	if (equActive)
	{
		while (pos < currentLine.size() && !isComment(currentLine,pos))
			pos++;

		std::wstring text = currentLine.substr(linePos,pos-linePos);
		createToken(TokenType::EquValue,pos-linePos,text);

		equActive = false;
		return token;
	}

	if (parseOperator())
		return token;

	// character constants
	wchar_t first = currentLine[pos];
	if (first == '\'' && pos+2 < currentLine.size() && currentLine[pos+2] == '\'')
	{
		createToken(TokenType::Integer,3,(u64)currentLine[pos+1]);
		return token;
	}

	// strings
	if (first == '"')
	{
		std::wstring text;
		pos++;

		bool valid = false;
		while (pos < currentLine.size())
		{
			if (pos+1 < currentLine.size() && currentLine[pos] == '\\' && currentLine[pos+1] == '"')
			{
				text += '"';
				pos += 2;
				continue;
			}

			if (currentLine[pos] == '"')
			{
				pos++;
				valid = true;
				break;
			}

			text += currentLine[pos++];
		}

		if (!valid)
		{
			createToken(TokenType::Invalid,pos-linePos,L"Unexpected end of line in string constant");
			return token;
		}
		
		createToken(TokenType::String,pos-linePos,text);
		return token;
	}

	// numbers
	if ((first >= '0' && first <= '9') || first == '$')
	{
		// find end of number
		size_t start = pos;
		size_t end = pos;
		bool isFloat = false;
		bool isValid = true;
		while (end < currentLine.size() && (iswalnum(currentLine[end]) || currentLine[end] == '.'))
		{
			if (currentLine[end] == '.')
			{
				if (isFloat == true)
					isValid = false;
				isFloat = true;
			}

			end++;
		}

		if (!isFloat)
		{
			u64 value;
			if (convertInteger(start,end,value) == false)
			{
				createToken(TokenType::NumberString,end-start,currentLine.substr(start,end-start));
				return token;
			}

			createToken(TokenType::Integer,end-start,value);
		} else { // isFloat
			double value;
			if (isValid == false)
			{
				createToken(TokenType::Invalid,end-start,L"Invalid floating point number");
				return token;
			}

			if (convertFloat(start,end,value) == false)
			{
				createToken(TokenType::NumberString,end-start,currentLine.substr(start,end-start));
				return token;
			}

			createToken(TokenType::Float,end-start,value);
		}
		
		return token;
	}

	// identifiers
	bool isFirst = true;
	while (pos < currentLine.size() && Global.symbolTable.isValidSymbolCharacter(currentLine[pos],isFirst))
	{
		pos++;
		isFirst = false;
	}

	if (pos == linePos)
	{
		std::wstring text = formatString(L"Invalid input '%c'",currentLine[pos]);
		createToken(TokenType::Invalid,1,text);
		return token;
	}

	std::wstring text = currentLine.substr(linePos,pos-linePos);
	std::transform(text.begin(), text.end(), text.begin(), ::towlower);

	if (text == L"equ")
	{
		createToken(TokenType::Equ,pos-linePos);
		equActive = true;
	} else {
		createToken(TokenType::Identifier,pos-linePos,text);
	}

	return token;
}

bool FileTokenizer::init(TextFile* input)
{
	currentLine.clear();
	lineNumber = 0;
	linePos = 0;
	equActive = false;

	this->input = input;
	if (input != NULL && input->isOpen())
	{
		skipWhitespace();
		return true;
	}

	return false;
}