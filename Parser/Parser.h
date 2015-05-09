#pragma once
#include "Tokenizer.h"
#include "Core/Expression.h"
#include "Commands/CommandSequence.h"
#include "DirectivesParser.h"
#include <set>
#include <map>

struct AssemblyTemplateArgument
{
	const wchar_t* variableName;
	std::wstring value;
};

struct ParserMacro
{
	std::wstring name;
	std::vector<std::wstring> parameters;
	std::set<std::wstring> labels;
	std::vector<Token> content;
	size_t counter;
};

class Parser
{
public:
	Parser();
	bool atEnd() { return entries.back()->atEnd(); }

	Expression parseExpression();
	bool parseExpressionList(std::vector<Expression>& list);
	bool parseIdentifier(std::wstring& dest);
	CAssemblerCommand* parseCommand();
	CAssemblerCommand* parseCommandSequence(wchar_t indicator = 0, std::initializer_list<wchar_t*> terminators = {});
	CAssemblerCommand* parseFile(TextFile& file);
	CAssemblerCommand* parseString(const std::wstring& text);
	CAssemblerCommand* parseTemplate(const std::wstring& text, std::initializer_list<AssemblyTemplateArgument> variables = {});
	CAssemblerCommand* parseDirective(const DirectiveEntry* directiveSet);
	bool matchToken(TokenType type, bool optional = false);

	Tokenizer* getTokenizer() { return entries.back(); };
	const Token& peekToken(int ahead = 0) { return getTokenizer()->peekToken(ahead); };
	const Token& nextToken() { return getTokenizer()->nextToken(); };
	void eatToken() { getTokenizer()->eatToken(); };
	void eatTokens(int num) { getTokenizer()->eatTokens(num); };
protected:
	CAssemblerCommand* Parser::parse(Tokenizer* tokenizer);
	CAssemblerCommand* parseLabel();
	bool parseMacro();
	bool checkEquLabel();
	bool checkMacroDefinition();
	CAssemblerCommand* parseMacroCall();

	std::vector<Tokenizer*> entries;
	std::map<std::wstring,ParserMacro> macros;
	std::set<std::wstring> macroLabels;
	bool initializingMacro;
};

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
	bool parse(Parser& parser, int& result);
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

bool checkExpressionListSize(std::vector<Expression>& list, int min, int max);
