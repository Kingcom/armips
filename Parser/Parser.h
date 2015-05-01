#pragma once
#include "Tokenizer.h"
#include "Core/Expression.h"
#include "Commands/CommandSequence.h"

struct TokenSequenceValue
{
	TokenSequenceValue(const wchar_t* text)
	{
		type = TokenType::Identifier;
		textValue = text;
	}
	
	TokenSequenceValue(u64 num)
	{
		type = TokenType::Integer;
		intValue = num;
	}
	
	TokenSequenceValue(double num)
	{
		type = TokenType::Float;
		floatValue = num;
	}
	

	TokenType type;
	union
	{
		const wchar_t* textValue;
		u64 intValue;
		double floatValue;
	};
};

typedef std::initializer_list<TokenType> TokenSequence;
typedef std::initializer_list<TokenSequenceValue> TokenValueSequence;

class TokenSequenceParser
{
public:
	void addEntry(int result, TokenSequence tokens, TokenValueSequence values);
	bool parse(Tokenizer& tokenizer, int& result);
	size_t getEntryCount() { return entries.size(); }
private:
	struct Entry
	{
		std::vector<TokenType> tokens;
		std::vector<TokenSequenceValue> values;
		int result;
	};

	std::vector<Entry> entries;
};

CAssemblerCommand* parseCommand(Tokenizer& tokenizer);
bool parseExpressionList(Tokenizer& tokenizer, std::vector<Expression>& list);
bool parseIdentifier(Tokenizer& tokenizer, std::wstring& dest);
bool checkExpressionListSize(std::vector<Expression>& list, int min, int max);
CommandSequence* parseCommandSequence(Tokenizer& tokenizer, std::initializer_list<wchar_t*> terminators);
CommandSequence* parseFile(TextFile& file);
CommandSequence* parseString(const std::wstring& text);

struct AssemblyTemplateArgument
{
	const wchar_t* variableName;
	std::wstring value;
};

CAssemblerCommand* parseTemplate(const std::wstring& text, std::initializer_list<AssemblyTemplateArgument> variables = {});
bool matchToken(Tokenizer& tokenizer, TokenType type, bool optional = false);
