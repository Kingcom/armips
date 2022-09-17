#include "Archs/MIPS/Mips.h"

#include "Archs/MIPS/MipsElfRelocator.h"
#include "Archs/MIPS/MipsExpressionFunctions.h"
#include "Archs/MIPS/MipsParser.h"

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

std::unique_ptr<CAssemblerCommand> CMipsArchitecture::parseDirective(Parser& parser)
{
	MipsParser mipsParser;
	return mipsParser.parseDirective(parser);
}

std::unique_ptr<CAssemblerCommand> CMipsArchitecture::parseOpcode(Parser& parser)
{
	MipsParser mipsParser;

	std::unique_ptr<CAssemblerCommand> macro = mipsParser.parseMacro(parser);
	if (macro != nullptr)
		return macro;

	return mipsParser.parseOpcode(parser);
}

void CMipsArchitecture::registerExpressionFunctions(ExpressionFunctionHandler &handler)
{
	registerMipsExpressionFunctions(handler);
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
	case MARCH_PSX:
		return std::make_unique<MipsElfRelocator>();
	case MARCH_RSP:
	default:
		return nullptr;
	}
}

void CMipsArchitecture::SetLoadDelay(bool Delay, int Register)
{
	LoadDelay = Delay;
	LoadDelayRegister = Register;
}
