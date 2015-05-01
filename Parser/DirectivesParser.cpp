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
	{ nullptr,				nullptr,						0 }
};

CAssemblerCommand* parseGlobalDirective(Tokenizer& tokenizer)
{
	return parseDirective(tokenizer,directives);
}
