#pragma once
#include "Util/FileClasses.h"

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
	LBrack,
	RBrack,
	Comma,
	Assign,
	Equ,
	EquValue,
	Hash,
	LBrace,
	RBrace,
	Dollar,
	NumberString,
	Degree
};

struct Token
{
	void setOriginalText(const std::wstring& t)
	{
		originalText = t;
	}

	void setOriginalText(const std::wstring& t, const size_t pos, const size_t len)
	{
		originalText = t.substr(pos, len);
	}

	std::wstring getOriginalText() const
	{
		return originalText;
	}

	void setStringValue(const std::wstring& t)
	{
		stringValue = t;
	}

	std::wstring getStringValue() const
	{
		return stringValue;
	}

	TokenType type;
	size_t line;
	size_t column;

	union
	{
		u64 intValue;
		double floatValue;
	};

protected:
	std::wstring originalText;
	std::wstring stringValue;
};

class Tokenizer
{
public:
	Tokenizer();
	Token& nextToken();
	Token& peekToken(int ahead = 0);
	void eatToken() { eatTokens(1); }
	void eatTokens(int num);
	bool atEnd() { return isInputAtEnd() && tokenIndex >= tokens.size(); }
	size_t getPosition() { return tokenIndex; }
	void setPosition(size_t pos) { tokenIndex = pos; }
	void skipLookahead();
	std::vector<Token> getTokens(size_t start, size_t count);
	void registerReplacement(const std::wstring& identifier, std::vector<Token>& tokens);
	void registerReplacement(const std::wstring& identifier, const std::wstring& newValue);
protected:
	virtual Token loadToken() = 0;
	virtual bool isInputAtEnd() = 0;
private:
	void readTokens(size_t maxIndex);
	std::vector<Token> tokens;
	size_t tokenIndex;

	struct Replacement
	{
		std::wstring identifier;
		std::vector<Token> value;
	};

	Token invalidToken;
	std::vector<Replacement> replacements;
};

class FileTokenizer: public Tokenizer
{
public:
	bool init(TextFile* input);
protected:
	virtual Token loadToken();
	virtual bool isInputAtEnd() { return linePos >= currentLine.size() && input->atEnd(); };

	void skipWhitespace();
	void createToken(TokenType type, size_t length);
	void createToken(TokenType type, size_t length, u64 value);
	void createToken(TokenType type, size_t length, double value);
	void createToken(TokenType type, size_t length, const std::wstring& value);

	bool convertInteger(size_t start, size_t end, u64& result);
	bool convertFloat(size_t start, size_t end, double& result);
	bool parseOperator();

	TextFile* input;
	std::wstring currentLine;
	size_t lineNumber;
	size_t linePos;
	
	Token token;
	bool equActive;
};

class TokenStreamTokenizer: public Tokenizer
{
public:
	void init(std::vector<Token>& tokens)
	{
		this->tokens = tokens;
		pos = 0;
	}
protected:
	virtual Token loadToken() { return tokens[pos++]; }
	virtual bool isInputAtEnd() { return pos == tokens.size(); }

	std::vector<Token> tokens;
	size_t pos;
};
