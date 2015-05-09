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
	Token() : originalText(nullptr), stringValue(nullptr)
	{
	}

	Token(Token &&src)
	{
		// Move strings.
		originalText = src.originalText;
		src.originalText = nullptr;
		stringValue = src.stringValue;
		src.stringValue = nullptr;

		// Just copy the rest.
		type = src.type;
		line = src.line;
		column = src.column;
		floatValue = src.floatValue;
	}

	Token(const Token &src) {
		// Copy strings.
		originalText = nullptr;
		if (src.originalText)
			setOriginalText(src.originalText);
		stringValue = nullptr;
		if (src.stringValue)
			setStringValue(src.stringValue);

		// And copy the rest.
		type = src.type;
		line = src.line;
		column = src.column;
		floatValue = src.floatValue;
	}

	~Token()
	{
		delete [] originalText;
		delete [] stringValue;
	}

	void setOriginalText(const std::wstring& t)
	{
		setOriginalText(t, 0, t.length());
	}

	void setOriginalText(const std::wstring& t, const size_t pos, const size_t len)
	{
		if (originalText)
			delete [] originalText;
		originalText = new wchar_t[len + 1];
		wmemcpy(originalText, t.data() + pos, len);
		originalText[len] = 0;
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
		if (stringValue)
			delete [] stringValue;
		stringValue = new wchar_t[len + 1];
		wmemcpy(stringValue, t.data() + pos, len);
		stringValue[len] = 0;
	}

	std::wstring getStringValue() const
	{
		if (stringValue)
			return stringValue;
		return L"";
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
	wchar_t* originalText;
	wchar_t* stringValue;
};

class Tokenizer
{
public:
	Tokenizer();
	const Token& nextToken();
	const Token& peekToken(int ahead = 0);
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
	void createToken(TokenType type, size_t length, const std::wstring& value, size_t valuePos, size_t valueLength);

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
	void init(const std::vector<Token>& tokens)
	{
		this->tokens.clear();
		for (const Token &tok: tokens)
			this->tokens.push_back(tok);
		pos = 0;
	}
protected:
	virtual Token loadToken() { return tokens[pos++]; }
	virtual bool isInputAtEnd() { return pos == tokens.size(); }

	std::vector<Token> tokens;
	size_t pos;
};
