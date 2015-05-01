#include "stdafx.h"
#include "Parser.h"
#include "ExpressionParser.h"
#include "Core/Misc.h"
#include "Commands/CommandSequence.h"
#include "Commands/CAssemblerLabel.h"
#include "Core/Common.h"
#include "DirectivesParser.h"

bool checkEquLabel(Tokenizer& tokenizer)
{
	if (tokenizer.peekToken(0).type == TokenType::Identifier)
	{
		int pos = 1;
		if (tokenizer.peekToken(pos).type == TokenType::Colon)
			pos++;

		if (tokenizer.peekToken(pos).type == TokenType::Equ &&
			tokenizer.peekToken(pos+1).type == TokenType::EquValue)
		{
			std::wstring name = tokenizer.peekToken(0).stringValue;
			std::wstring value = tokenizer.peekToken(pos+1).stringValue;
			tokenizer.eatTokens(pos+2);
		
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

CAssemblerCommand* parseLabel(Tokenizer& tokenizer, bool allLocal)
{
	if (tokenizer.peekToken(0).type == TokenType::Identifier &&
		tokenizer.peekToken(1).type == TokenType::Colon)
	{
		std::wstring name = tokenizer.peekToken(0).stringValue;
		tokenizer.eatTokens(2);

		if (allLocal == true && Global.symbolTable.isGlobalSymbol(name))
			name = L"@@" + name;
			
		return new CAssemblerLabel(name,Global.Section);
	}

	return nullptr;
}

CAssemblerCommand* parseCommand(Tokenizer& tokenizer)
{
	CAssemblerCommand* command;

	while (checkEquLabel(tokenizer) == true)
	{
		// do nothing, just parse all the equs there are
	}

	if (tokenizer.atEnd())
		return nullptr;

	if ((command = parseLabel(tokenizer,false)) != nullptr)
		return command;

	if ((command = Arch->parseDirective(tokenizer)) != nullptr)
		return command;

	if ((command = parseGlobalDirective(tokenizer)) != nullptr)
		return command;

	if ((command = Arch->parseOpcode(tokenizer)) != nullptr)
		return command;

	Logger::printError(Logger::Error,L"Parse error");
	tokenizer.eatToken();

	return nullptr;
}

bool parseExpressionList(Tokenizer& tokenizer, std::vector<Expression>& list)
{
	bool valid = true;
	list.clear();

	Expression exp = parseExpression(tokenizer);
	list.push_back(exp);

	if (exp.isLoaded() == false)
	{
		Logger::printError(Logger::Error,L"Parameter failure");
		tokenizer.skipLookahead();
		valid = false;
	}

	while (tokenizer.peekToken().type == TokenType::Comma)
	{
		tokenizer.eatToken();

		exp = parseExpression(tokenizer);
		list.push_back(exp);

		if (exp.isLoaded() == false)
		{
			Logger::printError(Logger::Error,L"Parameter failure");
			tokenizer.skipLookahead();
			valid = false;
		}
	}

	return valid;
}

bool parseIdentifier(Tokenizer& tokenizer, std::wstring& dest)
{
	Token tok = tokenizer.nextToken();
	if (tok.type != TokenType::Identifier)
		return false;

	dest = tok.stringValue;
	return true;
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

inline bool isPartOfList(const std::wstring& value, std::initializer_list<wchar_t*>& terminators)
{
	for (wchar_t* term: terminators)
	{
		if (value == term)
			return true;
	}

	return false;
}

CommandSequence* parseCommandSequence(Tokenizer& tokenizer, std::initializer_list<wchar_t*> terminators)
{
	CommandSequence* sequence = new CommandSequence();

	while (tokenizer.atEnd() == false && isPartOfList(tokenizer.peekToken().stringValue,terminators) == false)
	{
		CAssemblerCommand* cmd = parseCommand(tokenizer);
		sequence->addCommand(cmd);
	}

	return sequence;
}

CommandSequence* parseFile(TextFile& file)
{
	FileTokenizer tokenizer;
	if (tokenizer.init(&file) == false)
		return nullptr;

	return parseCommandSequence(tokenizer,{});
}

CommandSequence* parseString(const std::wstring& text)
{
	TextFile file;
	file.openMemory(text);
	return parseFile(file);
}
