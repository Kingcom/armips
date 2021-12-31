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

inline bool isPartOfList(const std::wstring& value, const std::initializer_list<const wchar_t*>& terminators)
{
	for (const wchar_t* term: terminators)
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

void Parser::printError(const Token &token, const std::wstring &text)
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
		printError(start,L"Parameter failure");
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
			printError(start,L"Parameter failure");
			getTokenizer()->skipLookahead();
			valid = false;
		}
	}

	if (list.size() < (size_t) min)
	{
		printError(start,L"Not enough parameters (min {})",min);
		return false;
	}

	if (max != -1 && (size_t) max < list.size())
	{
		printError(start,L"Too many parameters (max {})",max);
		return false;
	}

	return valid;
}

bool Parser::parseIdentifier(std::wstring& dest)
{
	const Token& tok = nextToken();
	if (tok.type != TokenType::Identifier)
		return false;

	dest = tok.getStringValue();
	return true;
}

std::unique_ptr<CAssemblerCommand> Parser::parseCommandSequence(wchar_t indicator, const std::initializer_list<const wchar_t*> terminators)
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

		if (next.stringValueStartsWith(indicator) && isPartOfList(next.getStringValue(), terminators))
		{
			foundTermination = true;
			break;
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
		std::wstring expected;
		for (const wchar_t* terminator : terminators)
		{
			if (!expected.empty())
				expected += L", ";
			expected += terminator;
		}

		Logger::printError(Logger::Error, L"Unterminated command sequence, expected any of {}.", expected);
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

std::unique_ptr<CAssemblerCommand> Parser::parseString(const std::wstring& text)
{
	TextFile file;
	file.openMemory(text);
	return parseFile(file,true);
}

std::unique_ptr<CAssemblerCommand> Parser::parseTemplate(const std::wstring& text, std::initializer_list<AssemblyTemplateArgument> variables)
{
	std::wstring fullText = text;

	overrideFileInfo = true;
	overrideFileNum = Global.FileInfo.FileNum;
	overrideLineNum = Global.FileInfo.LineNumber;

	for (auto& arg: variables)
	{
		size_t count = replaceAll(fullText,arg.variableName,arg.value);
		(void)count;
#ifdef _DEBUG
		if (count != 0 && arg.value.empty())
			Logger::printError(Logger::Warning,L"Empty replacement for {}",arg.variableName);
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

	const std::wstring stringValue = tok.getStringValue();

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
				printError(tok,L"Directive parameter failure");
			return nullptr;
		} else if (!(directive.flags & DIRECTIVE_MANUALSEPARATOR) && nextToken().type != TokenType::Separator)
		{
			printError(tok,L"Directive not terminated");
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
		Logger::queueError(Logger::Error, L"Max include/recursion depth reached");
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

void Parser::addEquation(const Token& startToken, const std::wstring& name, const std::wstring& value)
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
		if (token.type == TokenType::Identifier && token.getStringValue() == name)
		{
			printError(startToken,L"Recursive equ definition for \"{}\" not allowed",name);
			return;
		}

		if (token.type == TokenType::Equ)
		{
			printError(startToken,L"equ value must not contain another equ instance");
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
			std::wstring name = peekToken(0).getStringValue();
			std::wstring value = peekToken(pos+1).getStringValue();
			eatTokens(pos+2);

			// skip the equ if it's inside a false conditional block
			if (!isInsideTrueBlock())
				return true;

			// equs can't be inside blocks whose condition can only be
			// evaluated during validation
			if (isInsideUnknownBlock())
			{
				printError(start,L"equ not allowed inside of block with non-trivial condition");
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
				printError(start,L"Invalid equation name \"{}\"",name);
				return true;
			}

			if (Global.symbolTable.symbolExists(name,Global.FileInfo.FileNum,Global.Section))
			{
				printError(start,L"Equation name \"{}\" already defined",name);
				return true;
			}

			addEquation(start,name,value);
			return true;
		}
	}

	return false;
}

bool Parser::parseFunctionDeclaration(std::wstring& name, std::vector<std::wstring>& parameters)
{
	const Token& first = peekToken();
	if (first.type != TokenType::Identifier)
		return false;

	name = nextToken().getStringValue();

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

		parameters.emplace_back(token.getStringValue());
	}

	return !atEnd() && nextToken().type == TokenType::RParen;
}

bool Parser::checkExpFuncDefinition()
{
	const Token& first = peekToken();
	if (first.type != TokenType::Identifier)
		return false;

	if (!first.stringValueStartsWith(L'.') || first.getStringValue() != L".expfunc")
		return false;

	eatToken();

	std::wstring functionName;
	std::vector<std::wstring> functionParameters;

	// load declarationn
	if (!parseFunctionDeclaration(functionName, functionParameters))
	{
		printError(first, L"Invalid expression function declaration");
		return false;
	}

	if (nextToken().type != TokenType::Comma)
	{
		printError(first, L"Invalid expression function declaration");
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
		printError(first, L"Expression function definition not allowed inside of block with non-trivial condition");
		return false;
	}

	// if we are in a known false block, don't define the function
	if (!isInsideTrueBlock())
		return true;

	if(nextToken().type != TokenType::Separator)
	{
		printError(first, L".expfunc directive not terminated");
		return false;
	}

	// duplicate check
	if (ExpressionFunctionHandler::instance().find(functionName))
	{
		printError(first, L"Expression function \"{}\" already declared", functionName);
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

	if (!first.stringValueStartsWith(L'.') || first.getStringValue() != L".macro")
		return false;

	eatToken();

	// nested macro definitions are not allowed
	if (initializingMacro)
	{
		printError(first,L"Nested macro definitions not allowed");
		while (!atEnd())
		{
			const Token& token = nextToken();
			if (token.type == TokenType::Identifier && token.getStringValue() == L".endmacro")
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
		std::wstring name;
		if (!parameters[i].evaluateIdentifier(name))
			return false;

		macro.parameters.push_back(name);
	}

	if(nextToken().type != TokenType::Separator)
	{
		printError(first,L"Macro directive not terminated");
		return false;
	}

	// load macro content

	TokenizerPosition start = getTokenizer()->getPosition();
	bool valid = false;
	while (!atEnd())
	{
		const Token& tok = nextToken();
		if (tok.type == TokenType::Identifier && tok.getStringValue() == L".endmacro")
		{
			valid = true;
			break;
		}
	}

	// Macros have to be defined at parse time, so they can't be defined in blocks
	// with non-trivial conditions
	if (isInsideUnknownBlock())
	{
		printError(first, L"Macro definition not allowed inside of block with non-trivial condition");
		return false;
	}

	// if we are in a known false block, don't define the macro
	if (!isInsideTrueBlock())
		return true;
	
	// duplicate check
	if (macros.find(macro.name) != macros.end())
	{
		printError(first, L"Macro \"{}\" already defined", macro.name);
		return false;
	}

	// no .endmacro, not valid
	if (!valid)
	{
		printError(first, L"Macro \"{}\" not terminated", macro.name);
		return true;
	}

	// get content
	TokenizerPosition end = getTokenizer()->getPosition().previous();
	macro.content = getTokenizer()->getTokens(start,end);

	if(nextToken().type != TokenType::Separator)
	{
		printError(first,L"Endmacro directive not terminated");
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
		printError(macroStart, L"Unbalanced parentheses in macro parameter");
		return std::nullopt;
	}

	TokenizerPosition endPos = getTokenizer()->getPosition();
	std::vector<Token> tokens = getTokenizer()->getTokens(startPos,endPos);
	if (tokens.size() == 0)
	{
		printError(macroStart, L"Empty macro argument");
		return std::nullopt;
	}

	return tokens;
}

std::unique_ptr<CAssemblerCommand> Parser::parseMacroCall()
{
	const Token& start = peekToken();
	if (start.type != TokenType::Identifier)
		return nullptr;

	auto it = macros.find(start.getStringValue());
	if (it == macros.end())
		return nullptr;

	ParserMacro& macro = it->second;
	eatToken();

	// create a token stream for the macro content,
	// registering replacements for parameter values
	TokenStreamTokenizer macroTokenizer;

	std::set<std::wstring> identifierParameters;
	for (size_t i = 0; i < macro.parameters.size(); i++)
	{
		if (peekToken().type == TokenType::Separator)
		{
			printError(start,L"Too few macro arguments ({} vs {})",i,macro.parameters.size());
			return nullptr;
		}

		if (i != 0)
		{
			if (nextToken().type != TokenType::Comma)
			{
				printError(start,L"Macro arguments not comma-separated");
				return nullptr;
			}
		}

		auto tokens = extractMacroParameter(start);
		if (!tokens)
			return nullptr;

		// remember any single identifier parameters for the label replacement
		if (tokens->size() == 1 && tokens->front().type == TokenType::Identifier)
			identifierParameters.insert(tokens->front().getStringValue());

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

		printError(start,L"Too many macro arguments ({} vs {})",count,macro.parameters.size());		
		return nullptr;
	}

	if(nextToken().type != TokenType::Separator)
	{
		printError(start,L"Macro call not terminated");
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
	for (const std::wstring& label: macro.labels)
	{
		// check if the label is using the name of a parameter
		// in that case, don't register a unique replacement
		if (identifierParameters.find(label) != identifierParameters.end())
			continue;

		// otherwise make sure the name is unique
		std::wstring fullName;
		if (Global.symbolTable.isLocalSymbol(label))
			fullName = fmt::format(L"@@{}_{}_{:08X}",macro.name,label.substr(2),macro.counter);
		else if (Global.symbolTable.isStaticSymbol(label))
			fullName = fmt::format(L"@{}_{}_{:08X}",macro.name,label.substr(1),macro.counter);
		else
			fullName = fmt::format(L"{}_{}_{:08X}",macro.name,label,macro.counter);

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
		const std::wstring name = start.getStringValue();
		eatTokens(2);
		
		if (initializingMacro)
			macroLabels.insert(name);
		
		if (!Global.symbolTable.isValidSymbolName(name))
		{
			printError(start,L"Invalid label name \"{}\"",name);
			return nullptr;
		}

		return std::make_unique<CAssemblerLabel>(name,start.getOriginalText());
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
	printError(token,L"Parse error '{}'",token.getOriginalText());
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
				if (values == entry.values.end() || values->textValue != token.getStringValue())
				{
					valid = false;
					break;
				}
				
				values++;
			} else if (type == TokenType::Integer)
			{
				if (values == entry.values.end() || values->intValue != token.intValue)
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
