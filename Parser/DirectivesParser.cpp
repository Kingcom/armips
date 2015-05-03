#include "stdafx.h"
#include "DirectivesParser.h"
#include "Core/Common.h"
#include "Commands/CDirectiveFile.h"
#include "Commands/CDirectiveData.h"
#include "Commands/CDirectiveConditional.h"
#include "Commands/CDirectiveMessage.h"
#include "Commands/CDirectiveFill.h"
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
	{ L".if",				&parseDirectiveConditional,		DIRECTIVE_COND_IF },
	{ L".ifdef",			&parseDirectiveConditional,		DIRECTIVE_COND_IFDEF },
	{ L".ifndef",			&parseDirectiveConditional,		DIRECTIVE_COND_IFNDEF },
	{ L".ifarm",			&parseDirectiveConditional,		DIRECTIVE_COND_IFARM },
	{ L".ifthumb",			&parseDirectiveConditional,		DIRECTIVE_COND_IFTHUMB },

	{ nullptr,				nullptr,						0 }
};

CAssemblerCommand* parseGlobalDirective(Tokenizer& tokenizer)
{
	return parseDirective(tokenizer,directives);
}
