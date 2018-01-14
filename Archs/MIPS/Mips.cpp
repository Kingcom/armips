#include "stdafx.h"
#include "Mips.h"
#include "MipsParser.h"
#include "MipsExpressionFunctions.h"
#include "MipsElfRelocator.h"

CMipsArchitecture Mips;

CMipsArchitecture::CMipsArchitecture()
{
	FixLoadDelay = false;
	IgnoreLoadDelay = false;
	LoadDelay = false;
	LoadDelayRegister = 0;
	DelaySlot = false;
	Version = MARCH_INVALID;
}

CAssemblerCommand* CMipsArchitecture::parseDirective(Parser& parser)
{
	MipsParser mipsParser;
	return mipsParser.parseDirective(parser);
}

CAssemblerCommand* CMipsArchitecture::parseOpcode(Parser& parser)
{
	MipsParser mipsParser;

	CAssemblerCommand* macro = mipsParser.parseMacro(parser);
	if (macro != nullptr)
		return macro;

	return mipsParser.parseOpcode(parser);
}

const ExpressionFunctionMap& CMipsArchitecture::getExpressionFunctions()
{
	return mipsExpressionFunctions;
}

void CMipsArchitecture::NextSection()
{
	LoadDelay = false;
	LoadDelayRegister = 0;
	DelaySlot = false;
}

void CMipsArchitecture::Revalidate()
{
	LoadDelay = false;
	LoadDelayRegister = 0;
	DelaySlot = false;
}

std::unique_ptr<IElfRelocator> CMipsArchitecture::getElfRelocator()
{
	switch (Version)
	{
	case MARCH_PS2:
	case MARCH_PSP:
	case MARCH_N64:
		return std::unique_ptr<IElfRelocator>(new MipsElfRelocator());
	case MARCH_PSX:
	case MARCH_RSP:
	default:
		return NULL;
	}
}

void CMipsArchitecture::SetLoadDelay(bool Delay, int Register)
{
	LoadDelay = Delay;
	LoadDelayRegister = Register;
}
