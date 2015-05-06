#include "stdafx.h"
#include "Parser.h"
#include "ExpressionParser.h"
#include "Core/Misc.h"
#include "Commands/CommandSequence.h"
#include "Commands/CAssemblerLabel.h"
#include "Core/Common.h"
#include "Util/Util.h"

inline bool isPartOfList(const std::wstring& value, std::initializer_list<wchar_t*>& terminators)
{
	for (wchar_t* term: terminators)
	{
		if (value == term)
			return true;
	}

	return false;
}

Expression Parser::parseExpression()
{
	return ::parseExpression(*getTokenizer());
}

bool Parser::parseExpressionList(std::vector<Expression>& list)
{
	bool valid = true;
	list.clear();

	Expression exp = parseExpression();
	list.push_back(exp);

	if (exp.isLoaded() == false)
	{
		Logger::printError(Logger::Error,L"Parameter failure");
		getTokenizer()->skipLookahead();
		valid = false;
	}

	while (peekToken().type == TokenType::Comma)
	{
		eatToken();

		exp = parseExpression();
		list.push_back(exp);

		if (exp.isLoaded() == false)
		{
			Logger::printError(Logger::Error,L"Parameter failure");
			getTokenizer()->skipLookahead();
			valid = false;
		}
	}

	return valid;
}

bool Parser::parseIdentifier(std::wstring& dest)
{
	Token& tok = nextToken();
	if (tok.type != TokenType::Identifier)
		return false;

	dest = tok.stringValue;
	return true;
}

CAssemblerCommand* Parser::parseCommandSequence(std::initializer_list<wchar_t*> terminators)
{
	CommandSequence* sequence = new CommandSequence();

	while (atEnd() == false && isPartOfList(peekToken().stringValue,terminators) == false)
	{
		CAssemblerCommand* cmd = parseCommand();
		sequence->addCommand(cmd);
	}

	return sequence;
}

CAssemblerCommand* Parser::parseFile(TextFile& file)
{
	FileTokenizer tokenizer;
	if (tokenizer.init(&file) == false)
		return nullptr;

	return parse(&tokenizer,true,true);
}

CAssemblerCommand* Parser::parseString(const std::wstring& text)
{
	TextFile file;
	file.openMemory(text);
	return parseFile(file);
}

CAssemblerCommand* Parser::parseTemplate(const std::wstring& text, std::initializer_list<AssemblyTemplateArgument> variables)
{
	std::wstring fullText = text;

	for (auto& arg: variables)
	{
		replaceAll(fullText,arg.variableName,arg.value);
	}

	return parseString(fullText);
}

CAssemblerCommand* Parser::parseDirective(const DirectiveEntry* directiveSet)
{
	Token tok = peekToken();
	if (tok.type != TokenType::Identifier)
		return nullptr;

	for (size_t i = 0; directiveSet[i].name != nullptr; i++)
	{
		if (tok.stringValue == directiveSet[i].name)
		{
			if (directiveSet[i].flags & DIRECTIVE_DISABLED)
				continue;
			if ((directiveSet[i].flags & DIRECTIVE_NOCASHOFF) && Global.nocash == true)
				continue;
			if ((directiveSet[i].flags & DIRECTIVE_NOCASHON) && Global.nocash == false)
				continue;
			if ((directiveSet[i].flags & DIRECTIVE_NOTINMEMORY) && Global.memoryMode == true)
				continue;

			if (directiveSet[i].flags & DIRECTIVE_MIPSRESETDELAY)
				Arch->NextSection();

			eatToken();
			return directiveSet[i].function(*this,directiveSet[i].flags);
		}
	}

	return nullptr;
}

bool Parser::matchToken(TokenType type, bool optional)
{
	if (optional)
	{
		Token& token = peekToken();
		if (token.type == type)
			eatToken();
		return true;
	}
	
	return nextToken().type == type;
}

CAssemblerCommand* Parser::parse(Tokenizer* tokenizer, bool allowEqu, bool allowMacro)
{
	Entry entry;
	entry.tokenizer = tokenizer;
	entry.allowEqu = allowEqu;
	entry.allowMacro = allowMacro;
	entries.push_back(entry);

	CAssemblerCommand* sequence = parseCommandSequence();
	entries.pop_back();

	return sequence;
}

bool Parser::checkEquLabel()
{
	if (isEquAllowed() == false)
		return false;

	if (peekToken(0).type == TokenType::Identifier)
	{
		int pos = 1;
		if (peekToken(pos).type == TokenType::Colon)
			pos++;

		if (peekToken(pos).type == TokenType::Equ &&
			peekToken(pos+1).type == TokenType::EquValue)
		{
			std::wstring name = peekToken(0).stringValue;
			std::wstring value = peekToken(pos+1).stringValue;
			eatTokens(pos+2);
		
			if (Global.symbolTable.isValidSymbolName(name) == false)
			{
				Logger::printError(Logger::Error,L"Invalid equation name %s",name);
				return true;
			}

			if (Global.symbolTable.symbolExists(name,Global.FileInfo.FileNum,Global.Section))
			{
				Logger::printError(Logger::Error,L"Equation name %s already defined",name);
				return true;
			}
		
			Global.symbolTable.addEquation(name,Global.FileInfo.FileNum,Global.Section,value);
			return true;
		}
	}

	return false;
}

CAssemblerCommand* Parser::parseLabel()
{
	if (peekToken(0).type == TokenType::Identifier &&
		peekToken(1).type == TokenType::Colon)
	{
		std::wstring name = peekToken(0).stringValue;
		eatTokens(2);

		return new CAssemblerLabel(name);
	}

	return nullptr;
}

CAssemblerCommand* Parser::parseCommand()
{
	CAssemblerCommand* command;

	while (checkEquLabel())
	{
		// do nothing, just parse all the equs there are
	}

	if (atEnd())
		return nullptr;

	if ((command = parseLabel()) != nullptr)
		return command;

	if ((command = Arch->parseDirective(*this)) != nullptr)
		return command;

	if ((command = parseDirective(directives)) != nullptr)
		return command;

	if ((command = Arch->parseOpcode(*this)) != nullptr)
		return command;

	Logger::printError(Logger::Error,L"Parse error");
	eatToken();

	return nullptr;
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
		size_t pos = parser.getTokenizer()->getPosition();
		auto values = entry.values.begin();

		bool valid = true;
		for (TokenType type: entry.tokens)
		{
			// check of token type matches
			Token& token = parser.nextToken();
			if (token.type != type)
			{
				valid = false;
				break;
			}

			// if necessary, check if the value of the token also matches
			if (type == TokenType::Identifier)
			{
				if (values == entry.values.end() || values->textValue != token.stringValue)
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

bool checkExpressionListSize(std::vector<Expression>& list, int min, int max)
{
	if (list.size() < (size_t) min)
	{
		Logger::printError(Logger::Error,L"Not enough parameters (min %d)",min);
		return false;
	}

	if (max != -1 && (size_t) max < list.size())
	{
		Logger::printError(Logger::Error,L"Too many parameters (max %d)",max);
		return false;
	}

	return true;
}
