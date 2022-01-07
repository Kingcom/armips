#include "Parser/Parser.h"

#include "Archs/Architecture.h"
#include "Commands/CAssemblerLabel.h"
#include "Commands/CommandSequence.h"
#include "Core/Common.h"
#include "Core/ExpressionFunctionHandler.h"
#include "Core/Misc.h"
#include "Parser/DirectivesParser.h"
#include "Parser/ExpressionParser.h"
#include "Util/Util.h"

inline bool isPartOfList(const Identifier& value, const std::initializer_list<const char*>& terminators)
{
	for (const char* term: terminators)
	{
		if (value == term)
			return true;
	}

	return false;
}

Parser::Parser()
{
	initializingMacro = false;
	overrideFileInfo = false;
	conditionStack.push_back({true,false});
	clearError();
}

void Parser::pushConditionalResult(ConditionalResult cond)
{
	ConditionInfo info = conditionStack.back();
	info.inTrueBlock = info.inTrueBlock && cond != ConditionalResult::False;
	info.inUnknownBlock = info.inUnknownBlock || cond == ConditionalResult::Unknown;
	conditionStack.push_back(info);
}

void Parser::printError(const Token &token, const std::string &text)
{
	errorLine = token.line;
	Global.FileInfo.LineNumber = (int) token.line;
	Logger::printError(Logger::Error, text);
	error = true;
}

Expression Parser::parseExpression()
{
	return ::parseExpression(*getTokenizer(), !isInsideTrueBlock() || isInsideUnknownBlock());
}

bool Parser::parseExpressionList(std::vector<Expression>& list, int min, int max)
{
	bool valid = true;
	list.clear();
	list.reserve(max >= 0 ? max : 32);

	const Token& start = peekToken();

	Expression exp = parseExpression();
	list.push_back(exp);

	if (!exp.isLoaded())
	{
		printError(start, "Parameter failure");
		getTokenizer()->skipLookahead();
		valid = false;
	}

	while (peekToken().type == TokenType::Comma)
	{
		eatToken();

		exp = parseExpression();
		list.push_back(exp);

		if (!exp.isLoaded())
		{
			printError(start, "Parameter failure");
			getTokenizer()->skipLookahead();
			valid = false;
		}
	}

	if (list.size() < (size_t) min)
	{
		printError(start, "Not enough parameters (min %d)",min);
		return false;
	}

	if (max != -1 && (size_t) max < list.size())
	{
		printError(start, "Too many parameters (max %d)",max);
		return false;
	}

	return valid;
}

bool Parser::parseIdentifier(Identifier& dest)
{
	const Token& tok = nextToken();
	if (tok.type != TokenType::Identifier)
		return false;

	dest = tok.identifierValue();
	return true;
}

std::unique_ptr<CAssemblerCommand> Parser::parseCommandSequence(char indicator, const std::initializer_list<const char*> terminators)
{
	auto sequence = std::make_unique<CommandSequence>();

	bool foundTermination = false;
	while (!atEnd())
	{
		const Token &next = peekToken();

		if(next.type == TokenType::Separator)
		{
			eatToken();
			continue;
		}

		if (next.type == TokenType::Identifier)
		{
			const auto &identifier = next.identifierValue();
			if (identifier.startsWith(indicator) && isPartOfList(identifier, terminators))
			{
				foundTermination = true;
				break;
			}
		}

		bool foundSomething = false;
		while (checkEquLabel() || checkMacroDefinition() || checkExpFuncDefinition())
		{
			// do nothing, just parse all the equs and macros there are
			if (hasError())
				sequence->addCommand(handleError());

			foundSomething = true;
		}

		if (foundSomething)
			continue;

		std::unique_ptr<CAssemblerCommand> cmd = parseCommand();

		// omit commands inside blocks that are trivially false
		if (!isInsideTrueBlock())
		{
			continue;
		}

		sequence->addCommand(std::move(cmd));
	}

	if (!foundTermination && terminators.size())
	{
		std::string expected;
		for (const char* terminator : terminators)
		{
			if (!expected.empty())
				expected += ", ";
			expected += terminator;
		}

		Logger::printError(Logger::Error, "Unterminated command sequence, expected any of %s.", expected);
	}

	return sequence;
}

std::unique_ptr<CAssemblerCommand> Parser::parseFile(TextFile& file, bool virtualFile)
{
	FileTokenizer tokenizer;
	if (!tokenizer.init(&file))
		return nullptr;

	std::unique_ptr<CAssemblerCommand> result = parse(&tokenizer,virtualFile,file.getFileName());

	if (!file.isFromMemory())
		Global.FileInfo.TotalLineCount += file.getNumLines();

	return result;
}

std::unique_ptr<CAssemblerCommand> Parser::parseString(const std::string& text)
{
	TextFile file;
	file.openMemory(text);
	return parseFile(file,true);
}

std::unique_ptr<CAssemblerCommand> Parser::parseTemplate(const std::string& text, std::initializer_list<AssemblyTemplateArgument> variables)
{
	std::string fullText = text;

	overrideFileInfo = true;
	overrideFileNum = Global.FileInfo.FileNum;
	overrideLineNum = Global.FileInfo.LineNumber;

	for (auto& arg: variables)
	{
		size_t count = replaceAll(fullText,arg.variableName,arg.value);
		(void)count;
#ifdef _DEBUG
		if (count != 0 && arg.value.empty())
			Logger::printError(Logger::Warning, "Empty replacement for %s",arg.variableName);
#endif
	}

	std::unique_ptr<CAssemblerCommand> result = parseString(fullText);
	overrideFileInfo = false;

	return result;
}

std::unique_ptr<CAssemblerCommand> Parser::parseDirective(const DirectiveMap &directiveSet)
{
	const Token &tok = peekToken();
	if (tok.type != TokenType::Identifier)
		return nullptr;

	const std::string &stringValue = tok.identifierValue().string();

	auto matchRange = directiveSet.equal_range(stringValue);
	for (auto it = matchRange.first; it != matchRange.second; ++it)
	{
		const DirectiveEntry &directive = it->second;

		if (directive.flags & DIRECTIVE_DISABLED)
			continue;
		if ((directive.flags & DIRECTIVE_NOCASHOFF) && Global.nocash)
			continue;
		if ((directive.flags & DIRECTIVE_NOCASHON) && !Global.nocash)
			continue;
		if ((directive.flags & DIRECTIVE_NOTINMEMORY) && Global.memoryMode)
			continue;

		if (directive.flags & DIRECTIVE_MIPSRESETDELAY)
			Architecture::current().NextSection();

		eatToken();
		std::unique_ptr<CAssemblerCommand> result = directive.function(*this,directive.flags);
		if (result == nullptr)
		{
			if (!hasError())
				printError(tok, "Directive parameter failure");
			return nullptr;
		} else if (!(directive.flags & DIRECTIVE_MANUALSEPARATOR) && nextToken().type != TokenType::Separator)
		{
			printError(tok, "Directive not terminated");
			return nullptr;
		}

		return result;
	}

	return nullptr;
}

bool Parser::matchToken(TokenType type, bool optional)
{
	if (optional)
	{
		const Token& token = peekToken();
		if (token.type == type)
			eatToken();
		return true;
	}
	
	return nextToken().type == type;
}

std::unique_ptr<CAssemblerCommand> Parser::parse(Tokenizer* tokenizer, bool virtualFile, const fs::path& name)
{
	if (entries.size() >= 150)
	{
		Logger::queueError(Logger::Error, "Max include/recursion depth reached");
		return nullptr;
	}

	FileEntry entry;
	entry.tokenizer = tokenizer;
	entry.virtualFile = virtualFile;

	if (!virtualFile && !name.empty())
	{
		entry.fileNum = (int) Global.fileList.size();
		Global.fileList.add(name);
	} else {
		entry.fileNum = -1;
	}

	entries.push_back(entry);

	std::unique_ptr<CAssemblerCommand> sequence = parseCommandSequence();
	entries.pop_back();

	return sequence;
}

void Parser::addEquation(const Token& startToken, const Identifier& name, const std::string& value)
{
	// parse value string
	TextFile f;
	f.openMemory(value);

	FileTokenizer tok;
	tok.init(&f);

	TokenizerPosition start = tok.getPosition();
	while (!tok.atEnd() && tok.peekToken().type != TokenType::Separator)
	{
		const Token& token = tok.nextToken();
		if (token.type == TokenType::Identifier && token.identifierValue() == name)
		{
			printError(startToken, "Recursive equ definition for \"%s\" not allowed",name);
			return;
		}

		if (token.type == TokenType::Equ)
		{
			printError(startToken, "equ value must not contain another equ instance");
			return;
		}
	}

	// extract tokens
	TokenizerPosition end = tok.getPosition();
	std::vector<Token> tokens = tok.getTokens(start, end);
	size_t index = Tokenizer::addEquValue(tokens);

	for (FileEntry& entry : entries)
		entry.tokenizer->resetLookaheadCheckMarks();

	// register equation
	Global.symbolTable.addEquation(name, Global.FileInfo.FileNum, Global.Section, index);
}

bool Parser::checkEquLabel()
{
	updateFileInfo();

	const Token& start = peekToken();
	if (start.type == TokenType::Identifier)
	{
		int pos = 1;
		if (peekToken(pos).type == TokenType::Colon)
			pos++;

		if (peekToken(pos).type == TokenType::Equ &&
			peekToken(pos+1).type == TokenType::EquValue)
		{
			const Identifier &name = peekToken(0).identifierValue();
			const StringLiteral &value = peekToken(pos+1).stringValue();
			eatTokens(pos+2);

			// skip the equ if it's inside a false conditional block
			if (!isInsideTrueBlock())
				return true;

			// equs can't be inside blocks whose condition can only be
			// evaluated during validation
			if (isInsideUnknownBlock())
			{
				printError(start, "equ not allowed inside of block with non-trivial condition");
				return true;
			}

			// when parsing a macro, just remember the equ name
			if (initializingMacro)
			{
				macroLabels.insert(name);
				return true;
			}

			if (!Global.symbolTable.isValidSymbolName(name))
			{
				printError(start, "Invalid equation name \"%s\"",name);
				return true;
			}

			if (Global.symbolTable.symbolExists(name,Global.FileInfo.FileNum,Global.Section))
			{
				printError(start, "Equation name \"%s\" already defined",name);
				return true;
			}

			addEquation(start,name,value.string());
			return true;
		}
	}

	return false;
}

bool Parser::parseFunctionDeclaration(Identifier& name, std::vector<Identifier> &parameters)
{
	const Token& first = peekToken();
	if (first.type != TokenType::Identifier)
		return false;

	name = nextToken().identifierValue();

	if (nextToken().type != TokenType::LParen)
		return false;

	parameters.clear();
	while (!atEnd() && peekToken().type != TokenType::RParen)
	{
		if (!parameters.empty() && peekToken().type == TokenType::Comma)
			eatToken();

		const Token& token = nextToken();
		if (token.type != TokenType::Identifier)
			return false;

		parameters.emplace_back(token.identifierValue());
	}

	return !atEnd() && nextToken().type == TokenType::RParen;
}

bool Parser::checkExpFuncDefinition()
{
	const Token& first = peekToken();
	if (first.type != TokenType::Identifier)
		return false;

	const auto &identifier = first.identifierValue();
	if (!identifier.startsWith('.') || first.identifierValue() != ".expfunc")
		return false;

	eatToken();

	Identifier functionName;
	std::vector<Identifier> functionParameters;

	// load declarationn
	if (!parseFunctionDeclaration(functionName, functionParameters))
	{
		printError(first, "Invalid expression function declaration");
		return false;
	}

	if (nextToken().type != TokenType::Comma)
	{
		printError(first, "Invalid expression function declaration");
		return false;
	}

	// load definition
	TokenizerPosition start = getTokenizer()->getPosition();

	while (!atEnd() && peekToken().type != TokenType::Separator)
		eatToken();

	TokenizerPosition end = getTokenizer()->getPosition();
	auto functionContent = getTokenizer()->getTokens(start,end);

	// checks

	// Expression functions have to be defined at parse time, so they can't be defined in blocks
	// with non-trivial conditions
	if (isInsideUnknownBlock())
	{
		printError(first, "Expression function definition not allowed inside of block with non-trivial condition");
		return false;
	}

	// if we are in a known false block, don't define the function
	if (!isInsideTrueBlock())
		return true;

	if(nextToken().type != TokenType::Separator)
	{
		printError(first, ".expfunc directive not terminated");
		return false;
	}

	// duplicate check
	if (ExpressionFunctionHandler::instance().find(functionName))
	{
		printError(first, "Expression function \"%s\" already declared", functionName);
		return false;
	}

	// register function
	ExpressionFunctionHandler::instance().addUserFunction(functionName, functionParameters, functionContent);
	return true;
}

bool Parser::checkMacroDefinition()
{
	const Token& first = peekToken();
	if (first.type != TokenType::Identifier)
		return false;

	const auto &identifier = first.identifierValue();
	if (!identifier.startsWith('.') || first.identifierValue() != ".macro")
		return false;

	eatToken();

	// nested macro definitions are not allowed
	if (initializingMacro)
	{
		printError(first, "Nested macro definitions not allowed");
		while (!atEnd())
		{
			const Token& token = nextToken();
			if (token.type == TokenType::Identifier && token.identifierValue() == ".endmacro")
				break;
		}

		return true;
	}

	std::vector<Expression> parameters;
	if (!parseExpressionList(parameters,1,-1))
		return false;

	ParserMacro macro;
	macro.counter = 0;

	// load name
	if (!parameters[0].evaluateIdentifier(macro.name))
		return false;

	// load parameters
	for (size_t i = 1; i < parameters.size(); i++)
	{
		Identifier name;
		if (!parameters[i].evaluateIdentifier(name))
			return false;

		macro.parameters.push_back(name);
	}

	if(nextToken().type != TokenType::Separator)
	{
		printError(first, "Macro directive not terminated");
		return false;
	}

	// load macro content

	TokenizerPosition start = getTokenizer()->getPosition();
	bool valid = false;
	while (!atEnd())
	{
		const Token& tok = nextToken();
		if (tok.type == TokenType::Identifier && tok.identifierValue() == ".endmacro")
		{
			valid = true;
			break;
		}
	}

	// Macros have to be defined at parse time, so they can't be defined in blocks
	// with non-trivial conditions
	if (isInsideUnknownBlock())
	{
		printError(first, "Macro definition not allowed inside of block with non-trivial condition");
		return false;
	}

	// if we are in a known false block, don't define the macro
	if (!isInsideTrueBlock())
		return true;
	
	// duplicate check
	if (macros.find(macro.name) != macros.end())
	{
		printError(first, "Macro \"%s\" already defined", macro.name);
		return false;
	}

	// no .endmacro, not valid
	if (!valid)
	{
		printError(first, "Macro \"%s\" not terminated", macro.name);
		return true;
	}

	// get content
	TokenizerPosition end = getTokenizer()->getPosition().previous();
	macro.content = getTokenizer()->getTokens(start,end);

	if(nextToken().type != TokenType::Separator)
	{
		printError(first, "Endmacro directive not terminated");
		return false;
	}

	macros[macro.name] = macro;
	return true;
}

std::optional<std::vector<Token>> Parser::extractMacroParameter(const Token &macroStart)
{
	TokenizerPosition startPos = getTokenizer()->getPosition();

	// Find the end of the parameter. The parameter may contain expressions with function calls,
	// so keep track of the current parenthesis depth level
	int parenCount = 0;
	int braceCount = 0;
	int bracketCount = 0;

	while (peekToken().type != TokenType::Separator)
	{
		// if the next token is a comma, only exit the loop if parentheses are balanced
		auto type = peekToken().type;
		if (type == TokenType::Comma && parenCount == 0 && braceCount == 0 && bracketCount == 0)
			break;

		// keep track of parenthesis depth
		switch (type)
		{
		case TokenType::LParen:
			++parenCount;
			break;
		case TokenType::RParen:
			--parenCount;
			break;
		case TokenType::LBrace:
			++braceCount;
			break;
		case TokenType::RBrace:
			--braceCount;
			break;
		case TokenType::LBrack:
			++bracketCount;
			break;
		case TokenType::RBrack:
			--bracketCount;
			break;
		default:
			break;
		}

		eatToken();
	}

	if (parenCount != 0)
	{
		printError(macroStart, "Unbalanced parentheses in macro parameter");
		return std::nullopt;
	}

	TokenizerPosition endPos = getTokenizer()->getPosition();
	std::vector<Token> tokens = getTokenizer()->getTokens(startPos,endPos);
	if (tokens.size() == 0)
	{
		printError(macroStart, "Empty macro argument");
		return std::nullopt;
	}

	return tokens;
}

std::unique_ptr<CAssemblerCommand> Parser::parseMacroCall()
{
	const Token& start = peekToken();
	if (start.type != TokenType::Identifier)
		return nullptr;

	auto it = macros.find(start.identifierValue());
	if (it == macros.end())
		return nullptr;

	ParserMacro& macro = it->second;
	eatToken();

	// create a token stream for the macro content,
	// registering replacements for parameter values
	TokenStreamTokenizer macroTokenizer;

	std::set<Identifier> identifierParameters;
	for (size_t i = 0; i < macro.parameters.size(); i++)
	{
		if (peekToken().type == TokenType::Separator)
		{
			printError(start, "Too few macro arguments (%d vs %d)",i,macro.parameters.size());
			return nullptr;
		}

		if (i != 0)
		{
			if (nextToken().type != TokenType::Comma)
			{
				printError(start, "Macro arguments not comma-separated");
				return nullptr;
			}
		}

		auto tokens = extractMacroParameter(start);
		if (!tokens)
			return nullptr;

		// remember any single identifier parameters for the label replacement
		if (tokens->size() == 1 && tokens->front().type == TokenType::Identifier)
			identifierParameters.insert(tokens->front().identifierValue());

		// give them as a replacement to new tokenizer
		macroTokenizer.registerReplacement(macro.parameters[i], *tokens);
	}

	if (peekToken().type == TokenType::Comma)
	{
		size_t count = macro.parameters.size();
		while (peekToken().type == TokenType::Comma)
		{
			// skip comma
			eatToken();

			// skip parameter value
			extractMacroParameter(start);
			++count;
		}

		printError(start, "Too many macro arguments (%d vs %d)",count,macro.parameters.size());
		return nullptr;
	}

	if(nextToken().type != TokenType::Separator)
	{
		printError(start, "Macro call not terminated");
		return nullptr;
	}

	// skip macro instantiation in known false blocks
	if (!isInsideUnknownBlock() && !isInsideTrueBlock())
		return std::make_unique<DummyCommand>();

	// a macro is fully parsed once when it's loaded
	// to gather all labels. it's not necessary to
	// instantiate other macros at that time
	if (initializingMacro)
		return std::make_unique<DummyCommand>();

	// the first time a macro is instantiated, it needs to be analyzed
	// for labels
	if (macro.counter == 0)
	{
		initializingMacro = true;
		
		// parse the short lived next command
		macroTokenizer.init(macro.content);
		Logger::suppressErrors();
		std::unique_ptr<CAssemblerCommand> command =  parse(&macroTokenizer,true);
		Logger::unsuppressErrors();

		macro.labels = macroLabels;
		macroLabels.clear();
		
		initializingMacro = false;
	}

	// register labels and replacements
	for (const Identifier& label: macro.labels)
	{
		// check if the label is using the name of a parameter
		// in that case, don't register a unique replacement
		if (identifierParameters.find(label) != identifierParameters.end())
			continue;

		// otherwise make sure the name is unique
		std::string fullName;
		if (Global.symbolTable.isLocalSymbol(Identifier(label)))
			fullName = tfm::format("@@%s_%s_%08X",macro.name,label.string().substr(2),macro.counter);
		else if (Global.symbolTable.isStaticSymbol(Identifier(label)))
			fullName = tfm::format("@%s_%s_%08X",macro.name,label.string().substr(1),macro.counter);
		else
			fullName = tfm::format("%s_%s_%08X",macro.name,label,macro.counter);

		macroTokenizer.registerReplacement(label,fullName);
	}

	macroTokenizer.init(macro.content);
	macro.counter++;

	return parse(&macroTokenizer,true);

}

std::unique_ptr<CAssemblerCommand> Parser::parseLabel()
{
	updateFileInfo();

	const Token& start = peekToken(0);

	if (peekToken(0).type == TokenType::Identifier &&
		peekToken(1).type == TokenType::Colon)
	{
		const Identifier &name = start.identifierValue();
		eatTokens(2);
		
		if (initializingMacro)
			macroLabels.insert(name);
		
		if (!Global.symbolTable.isValidSymbolName(name))
		{
			printError(start, "Invalid label name \"%s\"",name);
			return nullptr;
		}

		return std::make_unique<CAssemblerLabel>(name,Identifier(start.getOriginalText()));
	}

	return nullptr;
}

std::unique_ptr<CAssemblerCommand> Parser::handleError()
{
	// skip the rest of the statement
	while (!atEnd() && nextToken().type != TokenType::Separator);

	clearError();
	return std::make_unique<InvalidCommand>();
}


void Parser::updateFileInfo()
{
	if (overrideFileInfo)
	{
		Global.FileInfo.FileNum = overrideFileNum;
		Global.FileInfo.LineNumber = overrideLineNum;
		return;
	}

	for (size_t i = entries.size(); i > 0; i--)
	{
		size_t index = i-1;

		if (!entries[index].virtualFile && entries[index].fileNum != -1)
		{
			Global.FileInfo.FileNum = entries[index].fileNum;

			// if it's not the topmost file, then the command to instantiate the
			// following files was already parsed -> take the previous command's line
			if (index != entries.size() - 1)
				Global.FileInfo.LineNumber = entries[index].previousCommandLine;
			else
			{
				Global.FileInfo.LineNumber = (int)entries[index].tokenizer->peekToken().line;
				entries[index].previousCommandLine = Global.FileInfo.LineNumber;
			}
			return;
		}
	}
}

std::unique_ptr<CAssemblerCommand> Parser::parseCommand()
{
	std::unique_ptr<CAssemblerCommand> command;

	updateFileInfo();

	if (atEnd())
		return std::make_unique<DummyCommand>();

	if ((command = parseLabel()) != nullptr)
		return command;
	if (hasError())
		return handleError();

	if ((command = parseMacroCall()) != nullptr)
		return command;
	if (hasError())
		return handleError();

	if ((command = Architecture::current().parseDirective(*this)) != nullptr)
		return command;
	if (hasError())
		return handleError();

	if ((command = parseDirective(directives)) != nullptr)
		return command;
	if (hasError())
		return handleError();

	if ((command = Architecture::current().parseOpcode(*this)) != nullptr)
		return command;
	if (hasError())
		return handleError();

	const Token& token = peekToken();
	printError(token,"Parse error '%s'", token.getOriginalText());
	return handleError();
}

void TokenSequenceParser::addEntry(int result, TokenSequence tokens, TokenValueSequence values)
{
	Entry entry = { tokens, values, result };
	entries.push_back(entry);
}

bool TokenSequenceParser::parse(Parser& parser, int& result)
{
	for (Entry& entry: entries)
	{
		TokenizerPosition pos = parser.getTokenizer()->getPosition();
		auto values = entry.values.begin();

		bool valid = true;
		for (TokenType type: entry.tokens)
		{
			// check of token type matches
			const Token& token = parser.nextToken();
			if (token.type != type)
			{
				valid = false;
				break;
			}

			// if necessary, check if the value of the token also matches
			if (type == TokenType::Identifier)
			{
				if (values == entry.values.end() || values->textValue != token.identifierValue())
				{
					valid = false;
					break;
				}
				
				values++;
			} else if (type == TokenType::Integer)
			{
				if (values == entry.values.end() || values->intValue != token.intValue())
				{
					valid = false;
					break;
				}
				
				values++;
			} 
		}

		if (valid && values == entry.values.end())
		{
			result = entry.result;
			return true;
		}

		parser.getTokenizer()->setPosition(pos);
	}

	return false;
}
