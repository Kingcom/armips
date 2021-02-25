#pragma once

#include <list>
#include <string>
#include <vector>

class TextFile;

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
	Degree,
	Separator
};

struct Token
{
	friend class Tokenizer;

	Token()
	{
	}

	void setOriginalText(const std::wstring& t)
	{
		setOriginalText(t, 0, t.length());
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
		setStringValue(t, 0, t.length());
	}

	void setStringValue(const std::wstring& t, const size_t pos, const size_t len)
	{
		stringValue = t.substr(pos, len);
	}

	void setStringAndOriginalValue(const std::wstring& t)
	{
		setStringAndOriginalValue(t, 0, t.length());
	}

	void setStringAndOriginalValue(const std::wstring& t, const size_t pos, const size_t len)
	{
		setStringValue(t, pos, len);
		originalText = stringValue;
	}

	std::wstring getStringValue() const
	{
		return stringValue;
	}

	bool stringValueStartsWith(wchar_t c) const
	{
		return stringValue[0] == c;
	}

	TokenType type;
	size_t line;
	size_t column;

	union
	{
		int64_t intValue;
		double floatValue;
	};

protected:
	std::wstring originalText;
	std::wstring stringValue;

	bool checked = false;
};

typedef std::list<Token> TokenList;

struct TokenizerPosition
{
	friend class Tokenizer;

	TokenizerPosition previous()
	{
		TokenizerPosition pos = *this;
		pos.it--;
		return pos;
	}
private:
	TokenList::iterator it;
};

class Tokenizer
{
public:
	Tokenizer();
	const Token& nextToken();
	const Token& peekToken(int ahead = 0);
	void eatToken() { eatTokens(1); }
	void eatTokens(int num);
	bool atEnd() { return position.it == tokens.end(); }
	TokenizerPosition getPosition() { return position; }
	void setPosition(TokenizerPosition pos) { position = pos; }
	void skipLookahead();
	std::vector<Token> getTokens(TokenizerPosition start, TokenizerPosition end) const;
	void registerReplacement(const std::wstring& identifier, std::vector<Token>& tokens);
	void registerReplacement(const std::wstring& identifier, const std::wstring& newValue);
	static size_t addEquValue(const std::vector<Token>& tokens);
	static void clearEquValues() { equValues.clear(); }
	void resetLookaheadCheckMarks();
protected:
	void clearTokens() { tokens.clear(); };
	void resetPosition() { position.it = tokens.begin(); } 
	void addToken(Token token);
private:
	bool processElement(TokenList::iterator& it);

	TokenList tokens;
	TokenizerPosition position;

	struct Replacement
	{
		std::wstring identifier;
		std::vector<Token> value;
	};

	Token invalidToken;
	std::vector<Replacement> replacements;
	static std::vector<std::vector<Token>> equValues;
};

class FileTokenizer: public Tokenizer
{
public:
	bool init(TextFile* input);
protected:
	Token loadToken();
	bool isInputAtEnd();

	void skipWhitespace();
	void createToken(TokenType type, size_t length);
	void createToken(TokenType type, size_t length, int64_t value);
	void createToken(TokenType type, size_t length, double value);
	void createToken(TokenType type, size_t length, const std::wstring& value);
	void createToken(TokenType type, size_t length, const std::wstring& value, size_t valuePos, size_t valueLength);
	void createTokenCurrentString(TokenType type, size_t length);

	bool convertInteger(size_t start, size_t end, int64_t& result);
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
	void init(const std::vector<Token>& tokens)
	{
		clearTokens();

		for (const Token &tok: tokens)
			addToken(tok);
		
		resetPosition();
	}
};
