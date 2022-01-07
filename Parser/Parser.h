#pragma once

#include "Core/Misc.h"
#include "Parser/Tokenizer.h"

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <tinyformat.h>

class CAssemblerCommand;
class Expression;
class TextFile;

struct DirectiveEntry;

using DirectiveMap = std::unordered_multimap<std::string, const DirectiveEntry>;

struct AssemblyTemplateArgument
{
	const char* variableName;
	std::string value;
};

struct ParserMacro
{
	Identifier name;
	std::vector<Identifier> parameters;
	std::set<Identifier> labels;
	std::vector<Token> content;
	size_t counter;
};

enum class ConditionalResult { Unknown, True, False };

class Parser
{
public:
	Parser();
	bool atEnd() { return entries.back().tokenizer->atEnd(); }

	void addEquation(const Token& start, const Identifier& name, const std::string& value);

	Expression parseExpression();
	bool parseExpressionList(std::vector<Expression>& list, int min = -1, int max = -1);
	bool parseIdentifier(Identifier& dest);
	std::unique_ptr<CAssemblerCommand> parseCommand();
	std::unique_ptr<CAssemblerCommand> parseCommandSequence(char indicator = 0, const std::initializer_list<const char*> terminators = {});
	std::unique_ptr<CAssemblerCommand> parseFile(TextFile& file, bool virtualFile = false);
	std::unique_ptr<CAssemblerCommand> parseString(const std::string& text);
	std::unique_ptr<CAssemblerCommand> parseTemplate(const std::string& text, const std::initializer_list<AssemblyTemplateArgument> variables = {});
	std::unique_ptr<CAssemblerCommand> parseDirective(const DirectiveMap &directiveSet);
	bool matchToken(TokenType type, bool optional = false);

	Tokenizer* getTokenizer() { return entries.back().tokenizer; };
	const Token& peekToken(int ahead = 0) { return getTokenizer()->peekToken(ahead); };
	const Token& nextToken() { return getTokenizer()->nextToken(); };
	void eatToken() { getTokenizer()->eatToken(); };
	void eatTokens(int num) { getTokenizer()->eatTokens(num); };
	
	void pushConditionalResult(ConditionalResult cond);
	void popConditionalResult() { conditionStack.pop_back(); };
	bool isInsideTrueBlock() { return conditionStack.back().inTrueBlock; }
	bool isInsideUnknownBlock() { return conditionStack.back().inUnknownBlock; }

	void printError(const Token &token, const std::string &text);

	template <typename... Args>
	void printError(const Token& token, const char* text, const Args&... args)
	{
		printError(token, tfm::format(text,args...));
	}

	bool hasError() { return error; }
	void updateFileInfo();
protected:
	void clearError() { error = false; }
	std::unique_ptr<CAssemblerCommand> handleError();

	std::unique_ptr<CAssemblerCommand> parse(Tokenizer* tokenizer, bool virtualFile, const fs::path& name = {});
	std::unique_ptr<CAssemblerCommand> parseLabel();
	bool checkEquLabel();
	bool parseFunctionDeclaration(Identifier& name, std::vector<Identifier>& parameters);
	bool checkExpFuncDefinition();
	bool checkMacroDefinition();

	std::optional<std::vector<Token>> extractMacroParameter(const Token &macroStart);
	std::unique_ptr<CAssemblerCommand> parseMacroCall();

	struct FileEntry
	{
		Tokenizer* tokenizer;
		bool virtualFile;
		int fileNum;
		int previousCommandLine;
	};

	std::vector<FileEntry> entries;
	std::map<Identifier, ParserMacro> macros;
	std::set<Identifier> macroLabels;
	bool initializingMacro;
	bool error;
	size_t errorLine;

	bool overrideFileInfo;
	int overrideFileNum;
	int overrideLineNum;

	struct ConditionInfo
	{
		bool inTrueBlock;
		bool inUnknownBlock;
	};

	std::vector<ConditionInfo> conditionStack;
};

struct TokenSequenceValue
{
	TokenSequenceValue(const char* text)
	{
		type = TokenType::Identifier;
		textValue = text;
	}
	
	TokenSequenceValue(int64_t num)
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
		const char* textValue;
		int64_t intValue;
		double floatValue;
	};
};

using TokenSequence = std::initializer_list<TokenType>;
using TokenValueSequence = std::initializer_list<TokenSequenceValue>;

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
