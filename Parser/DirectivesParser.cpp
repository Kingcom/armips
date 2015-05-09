#include "stdafx.h"
#include "DirectivesParser.h"
#include "Core/Common.h"
#include "Commands/CDirectiveFile.h"
#include "Commands/CDirectiveData.h"
#include "Commands/CDirectiveConditional.h"
#include "Commands/CDirectiveMessage.h"
#include "Commands/CDirectiveArea.h"
#include "Commands/CAssemblerLabel.h"
#include "Commands/CommandSequence.h"
#include "Archs/MIPS/Mips.h"
#include "Archs/ARM/Arm.h"
#include "Core/Expression.h"
#include "Util/Util.h"

#include "Tokenizer.h"
#include "ExpressionParser.h"
#include <initializer_list>
#include <algorithm>
#include "Parser.h"

CAssemblerCommand* parseDirectiveOpen(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (parser.parseExpressionList(list) == false)
		return nullptr;

	u64 memoryAddress;
	std::wstring inputName, outputName;

	if (checkExpressionListSize(list,2,3) == false)
		return nullptr;

	if (list[0].evaluateString(inputName,false) == false)
		return nullptr;

	if (list.back().evaluateInteger(memoryAddress) == false)
		return nullptr;

	if (list.size() == 3)
	{
		if (list[1].evaluateString(outputName,false) == false)
			return false;
		
		CDirectiveFile* file = new CDirectiveFile();
		file->initCopy(inputName,outputName,memoryAddress);
		return file;
	} else {
		CDirectiveFile* file = new CDirectiveFile();
		file->initOpen(inputName,memoryAddress);
		return file;
	}
}

CAssemblerCommand* parseDirectiveCreate(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (parser.parseExpressionList(list) == false)
		return nullptr;

	u64 memoryAddress;
	std::wstring inputName, outputName;

	if (checkExpressionListSize(list,2,2) == false)
		return nullptr;

	if (list[0].evaluateString(inputName,false) == false)
		return nullptr;

	if (list.back().evaluateInteger(memoryAddress) == false)
		return nullptr;

	CDirectiveFile* file = new CDirectiveFile();
	file->initCreate(inputName,memoryAddress);
	return file;
}

CAssemblerCommand* parseDirectiveClose(Parser& parser, int flags)
{
	CDirectiveFile* file = new CDirectiveFile();
	file->initClose();
	return file;
}

CAssemblerCommand* parseDirectiveIncbin(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (parser.parseExpressionList(list) == false)
		return nullptr;
	
	if (checkExpressionListSize(list,1,3) == false)
		return nullptr;

	std::wstring fileName;
	if (list[0].evaluateString(fileName,false) == false)
		return nullptr;

	CDirectiveIncbin* incbin = new CDirectiveIncbin(fileName);
	if (list.size() >= 2)
		incbin->setStart(list[1]);

	if (list.size() == 3)
		incbin->setSize(list[2]);

	return incbin;
}

CAssemblerCommand* parseDirectivePosition(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (parser.parseExpressionList(list) == false)
		return nullptr;
	
	if (checkExpressionListSize(list,1,1) == false)
		return nullptr;

	u64 position;
	if (list[0].evaluateInteger(position) == false)
	{
		Logger::printError(Logger::Error,L"Invalid ram address");
		return nullptr;
	}

	switch (flags & DIRECTIVE_USERMASK)
	{
	case DIRECTIVE_POS_PHYSICAL:
		return new CDirectivePosition(CDirectivePosition::Physical,position);
	case DIRECTIVE_POS_VIRTUAL:
		return new CDirectivePosition(CDirectivePosition::Virtual,position);
	}

	return nullptr;
}

CAssemblerCommand* parseDirectiveAlignFill(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (parser.parseExpressionList(list) == false)
		return nullptr;
	
	if (checkExpressionListSize(list,1,2) == false)
		return nullptr;

	CDirectiveAlignFill::Mode mode;
	switch (flags & DIRECTIVE_USERMASK)
	{
	case DIRECTIVE_FILE_ALIGN:
		mode = CDirectiveAlignFill::Align;
		break;
	case DIRECTIVE_FILE_FILL:
		mode = CDirectiveAlignFill::Fill;
		break;
	default:
		return nullptr;
	}

	if (list.size() == 2)
		return new CDirectiveAlignFill(list[0],list[1],mode);
	else
		return new CDirectiveAlignFill(list[0],mode);
}

CAssemblerCommand* parseDirectiveHeaderSize(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (parser.parseExpressionList(list) == false)
		return nullptr;
	
	if (checkExpressionListSize(list,1,1) == false)
		return nullptr;

	u64 size;
	if (list[0].evaluateInteger(size) == false)
	{
		Logger::printError(Logger::FatalError,L"Invalid header size");
		return nullptr;
	}

	return new CDirectiveHeaderSize(size);
}

CAssemblerCommand* parseDirectiveObjImport(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (parser.parseExpressionList(list) == false)
		return nullptr;
	
	if (checkExpressionListSize(list,1,2) == false)
		return nullptr;

	std::wstring fileName;
	if (list[0].evaluateString(fileName,true) == false)
		return nullptr;

	if (list.size() == 2)
	{
		std::wstring ctorName;
		if (list[1].evaluateIdentifier(ctorName) == false)
			return nullptr;

		return new DirectiveObjImport(fileName,ctorName);
	}
	
	return new DirectiveObjImport(fileName);
}

CAssemblerCommand* parseDirectiveConditional(Parser& parser, int flags)
{
	std::wstring name;
	Expression exp;

	CDirectiveConditional* cond;

	switch (flags)
	{
	case DIRECTIVE_COND_IF:
		exp = parser.parseExpression();
		cond = new CDirectiveConditional(ConditionType::IF,exp);
		break;
	case DIRECTIVE_COND_IFDEF:
		if (parser.parseIdentifier(name) == false)
			return nullptr;		
		cond = new CDirectiveConditional(ConditionType::IFDEF,name);
		break;
	case DIRECTIVE_COND_IFNDEF:
		if (parser.parseIdentifier(name) == false)
			return nullptr;
		cond = new CDirectiveConditional(ConditionType::IFNDEF,name);
		break;
	case DIRECTIVE_COND_IFARM:
		cond = new CDirectiveConditional(ConditionType::IFARM);
		break;
	case DIRECTIVE_COND_IFTHUMB:
		cond = new CDirectiveConditional(ConditionType::IFTHUMB);
		break;
	}

	CAssemblerCommand* ifBlock = parser.parseCommandSequence(L'.', {L".else", L".elseif", L".elseifdef", L".elseifndef", L".endif"});
	
	CAssemblerCommand* elseBlock = nullptr;
	const Token &next = parser.nextToken();
	const std::wstring stringValue = next.getStringValue();

	if (stringValue == L".else")
	{
		elseBlock = parser.parseCommandSequence(L'.', {L".endif"});
		parser.eatToken();	// eat .endif
	} else if (stringValue == L".elseif")
	{
		elseBlock = parseDirectiveConditional(parser,DIRECTIVE_COND_IF);
	} else if (stringValue == L".elseifdef")
	{
		elseBlock = parseDirectiveConditional(parser,DIRECTIVE_COND_IFDEF);
	} else if (stringValue == L".elseifndef")
	{
		elseBlock = parseDirectiveConditional(parser,DIRECTIVE_COND_IFNDEF);
	} else if (stringValue != L".endif")
	{
		return nullptr;
	}

	cond->setContent(ifBlock,elseBlock);
	return cond;
}

CAssemblerCommand* parseDirectiveTable(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (parser.parseExpressionList(list) == false)
		return nullptr;
	
	if (checkExpressionListSize(list,1,2) == false)
		return nullptr;

	std::wstring fileName;
	if (list[0].evaluateString(fileName,true) == false)
	{
		Logger::printError(Logger::Error,L"Invalid file name");
		return nullptr;
	}

	TextFile::Encoding encoding = TextFile::GUESS;
	if (list.size() == 2)
	{
		std::wstring encodingName;
		if (list[1].evaluateString(encodingName,true) == false)
		{
			Logger::printError(Logger::Error,L"Invalid encoding name");
			return nullptr;
		}

		encoding = getEncodingFromString(encodingName);
	}

	return new TableCommand(fileName,encoding);
}

CAssemblerCommand* parseDirectiveData(Parser& parser, int flags)
{
	bool terminate = false;
	if (flags & DIRECTIVE_DATA_TERMINATION)
	{
		terminate = true;
		flags &= ~DIRECTIVE_DATA_TERMINATION;
	}

	std::vector<Expression> list;
	if (parser.parseExpressionList(list) == false)
		return nullptr;
	
	if (checkExpressionListSize(list,1,-1) == false)
		return nullptr;

	CDirectiveData* data = new CDirectiveData();
	switch (flags & DIRECTIVE_USERMASK)
	{
	case DIRECTIVE_DATA_8:
		data->setNormal(list,1,false);
		break;
	case DIRECTIVE_DATA_16:
		data->setNormal(list,2,false);
		break;
	case DIRECTIVE_DATA_32:
		data->setNormal(list,4,false);
		break;
	case DIRECTIVE_DATA_ASCII:
		data->setNormal(list,1,true);
		break;
	case DIRECTIVE_DATA_SJIS:
		data->setSjis(list,terminate);
		break;
	case DIRECTIVE_DATA_CUSTOM:
		data->setCustom(list,terminate);
		break;
	}
	
	return data;
}

CAssemblerCommand* parseDirectivePsx(Parser& parser, int flags)
{
	Arch = &Mips;
	Mips.SetLoadDelay(false,0);
	Mips.SetVersion(MARCH_PSX);
	return new CommentCommand(L".psx",L"");
}

CAssemblerCommand* parseDirectivePs2(Parser& parser, int flags)
{
	Arch = &Mips;
	Mips.SetLoadDelay(false,0);
	Mips.SetVersion(MARCH_PS2);
	return new CommentCommand(L".ps2",L"");
}

CAssemblerCommand* parseDirectivePsp(Parser& parser, int flags)
{
	Arch = &Mips;
	Mips.SetLoadDelay(false,0);
	Mips.SetVersion(MARCH_PSP);
	return new CommentCommand(L".psp",L"");
}

CAssemblerCommand* parseDirectiveGba(Parser& parser, int flags)
{
	Arch = &Arm;
	Arm.SetThumbMode(true);
	Arm.SetArm9(false);
	return new CommentCommand(L".gba\n.thumb",L".thumb");
}

CAssemblerCommand* parseDirectiveNds(Parser& parser, int flags)
{
	Arch = &Arm;
	Arm.SetThumbMode(false);
	Arm.SetArm9(true);
	return new CommentCommand(L".nds\n.arm",L".arm");
}

CAssemblerCommand* parseDirectiveArea(Parser& parser, int flags)
{
	std::vector<Expression> parameters;
	if (parser.parseExpressionList(parameters) == false)
		return nullptr;
	
	bool valid = checkExpressionListSize(parameters,1,2);
	
	CAssemblerCommand* content = parser.parseCommandSequence(L'.', {L".endarea"});
	parser.eatToken();

	// area is invalid, return content anyway
	if (valid == false)
		return content;

	CDirectiveArea* area = new CDirectiveArea(content,parameters[0]);
	if (parameters.size() == 2)
		area->setFillExpression(parameters[1]);

	return area;
}

CAssemblerCommand* parseDirectiveErrorWarning(Parser& parser, int flags)
{
	const Token &tok = parser.nextToken();

	if (tok.type != TokenType::Identifier && tok.type != TokenType::String)
		return nullptr;

	std::wstring stringValue = tok.getStringValue();
	std::transform(stringValue.begin(),stringValue.end(),stringValue.begin(),::towlower);

	if (stringValue == L"on")
	{	
		Logger::setErrorOnWarning(true);
		return new DummyCommand();
	} else if (stringValue == L"off")
	{
		Logger::setErrorOnWarning(false);
		return new DummyCommand();
	}

	return nullptr;
}

CAssemblerCommand* parseDirectiveRelativeInclude(Parser& parser, int flags)
{
	const Token &tok = parser.nextToken();

	if (tok.type != TokenType::Identifier && tok.type != TokenType::String)
		return nullptr;

	std::wstring stringValue = tok.getStringValue();
	std::transform(stringValue.begin(),stringValue.end(),stringValue.begin(),::towlower);

	if (stringValue == L"on")
	{	
		Global.relativeInclude = true;
		return new DummyCommand();
	} else if (stringValue == L"off")
	{
		Global.relativeInclude = false;
		return new DummyCommand();
	}

	return nullptr;
}

CAssemblerCommand* parseDirectiveNocash(Parser& parser, int flags)
{
	const Token &tok = parser.nextToken();

	if (tok.type != TokenType::Identifier && tok.type != TokenType::String)
		return nullptr;

	std::wstring stringValue = tok.getStringValue();
	std::transform(stringValue.begin(),stringValue.end(),stringValue.begin(),::towlower);

	if (stringValue == L"on")
	{	
		Global.nocash = true;
		return new DummyCommand();
	} else if (stringValue == L"off")
	{
		Global.nocash = false;
		return new DummyCommand();
	}

	return nullptr;
}

CAssemblerCommand* parseDirectiveSym(Parser& parser, int flags)
{
	const Token &tok = parser.nextToken();

	if (tok.type != TokenType::Identifier && tok.type != TokenType::String)
		return nullptr;

	std::wstring stringValue = tok.getStringValue();
	std::transform(stringValue.begin(),stringValue.end(),stringValue.begin(),::towlower);

	if (stringValue == L"on")
		return new CDirectiveSym(true);
	else if (stringValue == L"off")
		return new CDirectiveSym(false);
	else
		return nullptr;
}

CAssemblerCommand* parseDirectiveDefineLabel(Parser& parser, int flags)
{
	// Cannot be a reference, we read more tokens below.
	const Token tok = parser.nextToken();
	if (tok.type != TokenType::Identifier)
		return nullptr;

	if (parser.nextToken().type != TokenType::Comma)
		return nullptr;

	Expression value = parser.parseExpression();
	if (value.isLoaded() == false)
		return nullptr;

	const std::wstring stringValue = tok.getStringValue();
	if (Global.symbolTable.isValidSymbolName(stringValue) == false)
	{
		Logger::printError(Logger::Error,L"Invalid label name \"%s\"",stringValue);
		return false;
	}

	return new CAssemblerLabel(stringValue,value);
}

CAssemblerCommand* parseDirectiveFunction(Parser& parser, int flags)
{
	std::vector<Expression> parameters;
	if (parser.parseExpressionList(parameters) == false)
		return nullptr;
	
	if (checkExpressionListSize(parameters,1,1) == false)
		return nullptr;

	std::wstring name;
	if (parameters[0].evaluateIdentifier(name) == false)
		return nullptr;

	CAssemblerCommand* seq = parser.parseCommandSequence(L'.', {L".endfunc",L".endfunction",L".func",L".function"});

	const std::wstring stringValue = parser.peekToken().getStringValue();
	if (stringValue == L".endfunc" ||
		stringValue == L".endfunction")
	{
		parser.eatToken();
	}

	return new CDirectiveFunction(name,seq);
}

CAssemblerCommand* parseDirectiveMessage(Parser& parser, int flags)
{
	Expression exp = parser.parseExpression();
	
	switch (flags)
	{
	case DIRECTIVE_MSG_WARNING:
		return new CDirectiveMessage(CDirectiveMessage::Type::Warning,exp);
	case DIRECTIVE_MSG_ERROR:
		return new CDirectiveMessage(CDirectiveMessage::Type::Error,exp);
	case DIRECTIVE_MSG_NOTICE:
		return new CDirectiveMessage(CDirectiveMessage::Type::Notice,exp);
	}

	return nullptr;
}

CAssemblerCommand* parseDirectiveInclude(Parser& parser, int flags)
{
	std::vector<Expression> parameters;
	if (parser.parseExpressionList(parameters) == false)
		return nullptr;
	
	if (checkExpressionListSize(parameters,1,2) == false)
		return nullptr;

	std::wstring fileName;
	if (parameters[0].evaluateString(fileName,true) == false)
		return nullptr;

	fileName = getFullPathName(fileName);

	TextFile::Encoding encoding = TextFile::GUESS;
	if (parameters.size() == 2)
	{
		std::wstring encodingName;
		if (parameters[1].evaluateString(encodingName,true) == false
			&& parameters[1].evaluateIdentifier(encodingName) == false)
			return nullptr;
		
		encoding = getEncodingFromString(encodingName);
	}

	if (fileExists(fileName) == false)
	{
		Logger::printError(Logger::Error,L"Included file \"%s\" does not exist",fileName);
		return nullptr;
	}

	TextFile f;
	if (f.open(fileName,TextFile::Read,encoding) == false)
	{
		Logger::printError(Logger::Error,L"Could not open included file \"%s\"",fileName);
		return nullptr;
	}

	return parser.parseFile(f);
}

const DirectiveMap directives = {
	{ L".open",				{ &parseDirectiveOpen,				DIRECTIVE_NOTINMEMORY } },
	{ L".openfile",			{ &parseDirectiveOpen,				DIRECTIVE_NOTINMEMORY } },
	{ L".create",			{ &parseDirectiveCreate,			DIRECTIVE_NOTINMEMORY } },
	{ L".createfile",		{ &parseDirectiveCreate,			DIRECTIVE_NOTINMEMORY } },
	{ L".close",			{ &parseDirectiveClose,				DIRECTIVE_NOTINMEMORY } },
	{ L".closefile",		{ &parseDirectiveClose,				DIRECTIVE_NOTINMEMORY } },
	{ L".incbin",			{ &parseDirectiveIncbin,			0 } },
	{ L".import",			{ &parseDirectiveIncbin,			0 } },
	{ L".org",				{ &parseDirectivePosition,			DIRECTIVE_POS_VIRTUAL } },
	{ L"org",				{ &parseDirectivePosition,			DIRECTIVE_POS_VIRTUAL } },
	{ L".orga",				{ &parseDirectivePosition,			DIRECTIVE_POS_PHYSICAL } },
	{ L"orga",				{ &parseDirectivePosition,			DIRECTIVE_POS_PHYSICAL } },
	{ L".align",			{ &parseDirectiveAlignFill,			DIRECTIVE_FILE_ALIGN } },
	{ L".fill",				{ &parseDirectiveAlignFill,			DIRECTIVE_FILE_FILL } },
	{ L"defs",				{ &parseDirectiveAlignFill,			DIRECTIVE_FILE_FILL } },
	{ L".headersize",		{ &parseDirectiveHeaderSize,		0 } },

	{ L".if",				{ &parseDirectiveConditional,		DIRECTIVE_COND_IF } },
	{ L".ifdef",			{ &parseDirectiveConditional,		DIRECTIVE_COND_IFDEF } },
	{ L".ifndef",			{ &parseDirectiveConditional,		DIRECTIVE_COND_IFNDEF } },
	{ L".ifarm",			{ &parseDirectiveConditional,		DIRECTIVE_COND_IFARM } },
	{ L".ifthumb",			{ &parseDirectiveConditional,		DIRECTIVE_COND_IFTHUMB } },

	{ L".loadtable",		{ &parseDirectiveTable,				0 } },
	{ L".table",			{ &parseDirectiveTable,				0 } },
	{ L".byte",				{ &parseDirectiveData,				DIRECTIVE_DATA_8 } },
	{ L".halfword",			{ &parseDirectiveData,				DIRECTIVE_DATA_16 } },
	{ L".word",				{ &parseDirectiveData,				DIRECTIVE_DATA_32 } },
	{ L".db",				{ &parseDirectiveData,				DIRECTIVE_DATA_8 } },
	{ L".dh",				{ &parseDirectiveData,				DIRECTIVE_DATA_16|DIRECTIVE_NOCASHOFF } },
	{ L".dw",				{ &parseDirectiveData,				DIRECTIVE_DATA_32|DIRECTIVE_NOCASHOFF } },
	{ L".dw",				{ &parseDirectiveData,				DIRECTIVE_DATA_16|DIRECTIVE_NOCASHON } },
	{ L".dd",				{ &parseDirectiveData,				DIRECTIVE_DATA_32|DIRECTIVE_NOCASHON } },
	{ L".dcb",				{ &parseDirectiveData,				DIRECTIVE_DATA_8 } },
	{ L".dcw",				{ &parseDirectiveData,				DIRECTIVE_DATA_16 } },
	{ L".dcd",				{ &parseDirectiveData,				DIRECTIVE_DATA_32 } },
	{ L"db",				{ &parseDirectiveData,				DIRECTIVE_DATA_8 } },
	{ L"dh",				{ &parseDirectiveData,				DIRECTIVE_DATA_16|DIRECTIVE_NOCASHOFF } },
	{ L"dw",				{ &parseDirectiveData,				DIRECTIVE_DATA_32|DIRECTIVE_NOCASHOFF } },
	{ L"dw",				{ &parseDirectiveData,				DIRECTIVE_DATA_16|DIRECTIVE_NOCASHON } },
	{ L"dd",				{ &parseDirectiveData,				DIRECTIVE_DATA_32|DIRECTIVE_NOCASHON } },
	{ L"dcb",				{ &parseDirectiveData,				DIRECTIVE_DATA_8 } },
	{ L"dcw",				{ &parseDirectiveData,				DIRECTIVE_DATA_16 } },
	{ L"dcd",				{ &parseDirectiveData,				DIRECTIVE_DATA_32 } },
	{ L".ascii",			{ &parseDirectiveData,				DIRECTIVE_DATA_8|DIRECTIVE_DATA_ASCII } },
	{ L".string",			{ &parseDirectiveData,				DIRECTIVE_DATA_CUSTOM|DIRECTIVE_DATA_TERMINATION } },
	{ L".str",				{ &parseDirectiveData,				DIRECTIVE_DATA_CUSTOM|DIRECTIVE_DATA_TERMINATION } },
	{ L".stringn",			{ &parseDirectiveData,				DIRECTIVE_DATA_CUSTOM } },
	{ L".strn",				{ &parseDirectiveData,				DIRECTIVE_DATA_CUSTOM } },
	{ L".sjis",				{ &parseDirectiveData,				DIRECTIVE_DATA_SJIS|DIRECTIVE_DATA_TERMINATION } },
	{ L".sjisn",			{ &parseDirectiveData,				DIRECTIVE_DATA_SJIS } },

	{ L".psx",				{ &parseDirectivePsx,				0 } },
	{ L".ps2",				{ &parseDirectivePs2,				0 } },
	{ L".psp",				{ &parseDirectivePsp,				0 } },
	{ L".gba",				{ &parseDirectiveGba,				0 } },
	{ L".nds",				{ &parseDirectiveNds,				0 } },

	{ L".area",				{ &parseDirectiveArea,				0 } },

	{ L".importobj",		{ &parseDirectiveObjImport,			0 } },
	{ L".importlib",		{ &parseDirectiveObjImport,			0 } },

	{ L".erroronwarning",	{ &parseDirectiveErrorWarning,		0 } },
	{ L".relativeinclude",	{ &parseDirectiveRelativeInclude,	0 } },
	{ L".nocash",			{ &parseDirectiveNocash,			0 } },
	{ L".sym",				{ &parseDirectiveSym,				0 } },
	
	{ L".definelabel",		{ &parseDirectiveDefineLabel,		0 } },
	{ L".function",			{ &parseDirectiveFunction,			0 } },
	{ L".func",				{ &parseDirectiveFunction,			0 } },
	
	{ L".warning",			{ &parseDirectiveMessage,			DIRECTIVE_MSG_WARNING } },
	{ L".error",			{ &parseDirectiveMessage,			DIRECTIVE_MSG_ERROR } },
	{ L".notice",			{ &parseDirectiveMessage,			DIRECTIVE_MSG_NOTICE } },

	{ L".include",			{ &parseDirectiveInclude,			0 } },
};
