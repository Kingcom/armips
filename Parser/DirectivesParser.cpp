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

CAssemblerCommand* parseDirectiveOpen(Tokenizer& tokenizer, int flags)
{
	std::vector<Expression> list;
	if (parseExpressionList(tokenizer,list) == false)
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

CAssemblerCommand* parseDirectiveCreate(Tokenizer& tokenizer, int flags)
{
	std::vector<Expression> list;
	if (parseExpressionList(tokenizer,list) == false)
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

CAssemblerCommand* parseDirectiveClose(Tokenizer& tokenizer, int flags)
{
	CDirectiveFile* file = new CDirectiveFile();
	file->initClose();
	return file;
}

CAssemblerCommand* parseDirectiveIncbin(Tokenizer& tokenizer, int flags)
{
	std::vector<Expression> list;
	if (parseExpressionList(tokenizer,list) == false)
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

CAssemblerCommand* parseDirectivePosition(Tokenizer& tokenizer, int flags)
{
	std::vector<Expression> list;
	if (parseExpressionList(tokenizer,list) == false)
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

CAssemblerCommand* parseDirectiveAlignFill(Tokenizer& tokenizer, int flags)
{
	std::vector<Expression> list;
	if (parseExpressionList(tokenizer,list) == false)
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

CAssemblerCommand* parseDirectiveHeaderSize(Tokenizer& tokenizer, int flags)
{
	std::vector<Expression> list;
	if (parseExpressionList(tokenizer,list) == false)
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

CAssemblerCommand* parseDirectiveConditional(Tokenizer& tokenizer, int flags)
{
	std::wstring name;
	Expression exp;

	CDirectiveConditional* cond;

	switch (flags)
	{
	case DIRECTIVE_COND_IF:
		exp = parseExpression(tokenizer);
		cond = new CDirectiveConditional(ConditionType::IF,exp);
		break;
	case DIRECTIVE_COND_IFDEF:
		if (parseIdentifier(tokenizer,name) == false)
			return nullptr;		
		cond = new CDirectiveConditional(ConditionType::IFDEF,name);
		break;
	case DIRECTIVE_COND_IFNDEF:
		if (parseIdentifier(tokenizer,name) == false)
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

	CommandSequence* ifBlock = parseCommandSequence(tokenizer,{L".else", L".elseif", L".elseifdef", L".elseifndef", L".endif"});
	
	CAssemblerCommand* elseBlock = nullptr;
	Token next = tokenizer.nextToken();

	if (next.stringValue == L".else")
	{
		elseBlock = parseCommandSequence(tokenizer,{L".endif"});
	} else if (next.stringValue == L".elseif")
	{
		elseBlock = parseDirectiveConditional(tokenizer,DIRECTIVE_COND_IF);
	} else if (next.stringValue == L".elseifdef")
	{
		elseBlock = parseDirectiveConditional(tokenizer,DIRECTIVE_COND_IFDEF);
	} else if (next.stringValue == L".elseifndef")
	{
		elseBlock = parseDirectiveConditional(tokenizer,DIRECTIVE_COND_IFNDEF);
	} else if (next.stringValue != L".endif")
	{
		return nullptr;
	}

	cond->setContent(ifBlock,elseBlock);
	return cond;
}

CAssemblerCommand* parseDirectiveTable(Tokenizer& tokenizer, int flags)
{
	std::vector<Expression> list;
	if (parseExpressionList(tokenizer,list) == false)
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

CAssemblerCommand* parseDirectiveData(Tokenizer& tokenizer, int flags)
{
	bool terminate = false;
	if (flags & DIRECTIVE_DATA_TERMINATION)
	{
		terminate = true;
		flags &= ~DIRECTIVE_DATA_TERMINATION;
	}

	std::vector<Expression> list;
	if (parseExpressionList(tokenizer,list) == false)
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

CAssemblerCommand* parseDirectivePsx(Tokenizer& tokenizer, int flags)
{
	Arch = &Mips;
	Mips.SetLoadDelay(false,0);
	Mips.SetVersion(MARCH_PSX);
	return new CommentCommand(L".psx",L"");
}

CAssemblerCommand* parseDirectivePs2(Tokenizer& tokenizer, int flags)
{
	Arch = &Mips;
	Mips.SetLoadDelay(false,0);
	Mips.SetVersion(MARCH_PS2);
	return new CommentCommand(L".ps2",L"");
}

CAssemblerCommand* parseDirectivePsp(Tokenizer& tokenizer, int flags)
{
	Arch = &Mips;
	Mips.SetLoadDelay(false,0);
	Mips.SetVersion(MARCH_PSP);
	return new CommentCommand(L".psp",L"");
}

CAssemblerCommand* parseDirectiveGba(Tokenizer& tokenizer, int flags)
{
	Arch = &Arm;
	Arm.SetThumbMode(true);
	Arm.SetArm9(false);
	return new CommentCommand(L".gba\n.thumb",L".thumb");
}

CAssemblerCommand* parseDirectiveNds(Tokenizer& tokenizer, int flags)
{
	Arch = &Arm;
	Arm.SetThumbMode(false);
	Arm.SetArm9(true);
	return new CommentCommand(L".nds\n.arm",L".arm");
}

CAssemblerCommand* parseDirective(Tokenizer& tokenizer, const DirectiveEntry* directiveSet)
{
	Token tok = tokenizer.peekToken();
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

			tokenizer.eatToken();
			return directiveSet[i].function(tokenizer,directiveSet[i].flags);
		}
	}

	return nullptr;
}

const DirectiveEntry directives[] = {
	{ L".open",				&parseDirectiveOpen,			DIRECTIVE_NOTINMEMORY },
	{ L".openfile",			&parseDirectiveOpen,			DIRECTIVE_NOTINMEMORY },
	{ L".create",			&parseDirectiveCreate,			DIRECTIVE_NOTINMEMORY },
	{ L".createfile",		&parseDirectiveCreate,			DIRECTIVE_NOTINMEMORY },
	{ L".close",			&parseDirectiveClose,			DIRECTIVE_NOTINMEMORY },
	{ L".closefile",		&parseDirectiveClose,			DIRECTIVE_NOTINMEMORY },
	{ L".incbin",			&parseDirectiveIncbin,			0 },
	{ L".import",			&parseDirectiveIncbin,			0 },
	{ L".org",				&parseDirectivePosition,		DIRECTIVE_POS_VIRTUAL },
	{ L"org",				&parseDirectivePosition,		DIRECTIVE_POS_VIRTUAL },
	{ L".orga",				&parseDirectivePosition,		DIRECTIVE_POS_PHYSICAL },
	{ L"orga",				&parseDirectivePosition,		DIRECTIVE_POS_PHYSICAL },
	{ L".align",			&parseDirectiveAlignFill,		DIRECTIVE_FILE_ALIGN },
	{ L".fill",				&parseDirectiveAlignFill,		DIRECTIVE_FILE_FILL },
	{ L"defs",				&parseDirectiveAlignFill,		DIRECTIVE_FILE_FILL },
	{ L".headersize",		&parseDirectiveHeaderSize,		0 },

	{ L".if",				&parseDirectiveConditional,		DIRECTIVE_COND_IF },
	{ L".ifdef",			&parseDirectiveConditional,		DIRECTIVE_COND_IFDEF },
	{ L".ifndef",			&parseDirectiveConditional,		DIRECTIVE_COND_IFNDEF },
	{ L".ifarm",			&parseDirectiveConditional,		DIRECTIVE_COND_IFARM },
	{ L".ifthumb",			&parseDirectiveConditional,		DIRECTIVE_COND_IFTHUMB },

	{ L".loadtable",		&parseDirectiveTable,			0 },
	{ L".table",			&parseDirectiveTable,			0 },
	{ L".byte",				&parseDirectiveData,			DIRECTIVE_DATA_8 },
	{ L".halfword",			&parseDirectiveData,			DIRECTIVE_DATA_16 },
	{ L".word",				&parseDirectiveData,			DIRECTIVE_DATA_32 },
	{ L".db",				&parseDirectiveData,			DIRECTIVE_DATA_8 },
	{ L".dh",				&parseDirectiveData,			DIRECTIVE_DATA_16|DIRECTIVE_NOCASHOFF },
	{ L".dw",				&parseDirectiveData,			DIRECTIVE_DATA_32|DIRECTIVE_NOCASHOFF },
	{ L".dw",				&parseDirectiveData,			DIRECTIVE_DATA_16|DIRECTIVE_NOCASHON },
	{ L".dd",				&parseDirectiveData,			DIRECTIVE_DATA_32|DIRECTIVE_NOCASHON },
	{ L".dcb",				&parseDirectiveData,			DIRECTIVE_DATA_8 },
	{ L".dcw",				&parseDirectiveData,			DIRECTIVE_DATA_16 },
	{ L".dcd",				&parseDirectiveData,			DIRECTIVE_DATA_32 },
	{ L"db",				&parseDirectiveData,			DIRECTIVE_DATA_8 },
	{ L"dh",				&parseDirectiveData,			DIRECTIVE_DATA_16|DIRECTIVE_NOCASHOFF },
	{ L"dw",				&parseDirectiveData,			DIRECTIVE_DATA_32|DIRECTIVE_NOCASHOFF },
	{ L"dw",				&parseDirectiveData,			DIRECTIVE_DATA_16|DIRECTIVE_NOCASHON },
	{ L"dd",				&parseDirectiveData,			DIRECTIVE_DATA_32|DIRECTIVE_NOCASHON },
	{ L"dcb",				&parseDirectiveData,			DIRECTIVE_DATA_8 },
	{ L"dcw",				&parseDirectiveData,			DIRECTIVE_DATA_16 },
	{ L"dcd",				&parseDirectiveData,			DIRECTIVE_DATA_32 },
	{ L".ascii",			&parseDirectiveData,			DIRECTIVE_DATA_8|DIRECTIVE_DATA_ASCII },
	{ L".string",			&parseDirectiveData,			DIRECTIVE_DATA_CUSTOM|DIRECTIVE_DATA_TERMINATION },
	{ L".str",				&parseDirectiveData,			DIRECTIVE_DATA_CUSTOM|DIRECTIVE_DATA_TERMINATION },
	{ L".stringn",			&parseDirectiveData,			DIRECTIVE_DATA_CUSTOM },
	{ L".strn",				&parseDirectiveData,			DIRECTIVE_DATA_CUSTOM },
	{ L".sjis",				&parseDirectiveData,			DIRECTIVE_DATA_SJIS|DIRECTIVE_DATA_TERMINATION },
	{ L".sjisn",			&parseDirectiveData,			DIRECTIVE_DATA_SJIS },

	{ L".psx",				&parseDirectivePsx,				0 },
	{ L".ps2",				&parseDirectivePs2,				0 },
	{ L".psp",				&parseDirectivePsp,				0 },
	{ L".gba",				&parseDirectiveGba,				0 },
	{ L".nds",				&parseDirectiveNds,				0 },

	{ nullptr,				nullptr,						0 }
};

CAssemblerCommand* parseGlobalDirective(Tokenizer& tokenizer)
{
	return parseDirective(tokenizer,directives);
}
