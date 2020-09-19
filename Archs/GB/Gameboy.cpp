#include "Archs/GB/CGameboyInstruction.h"
#include "Archs/GB/Gameboy.h"
#include "Archs/GB/GameboyParser.h"
#include "Parser/Parser.h"

CGameboyArchitecture Gameboy;

CGameboyArchitecture::CGameboyArchitecture()
{
}

std::unique_ptr<CAssemblerCommand> CGameboyArchitecture::parseDirective(Parser& parser)
{
	GameboyParser gameboyParser;

	return gameboyParser.parseDirective(parser);
}

std::unique_ptr<CAssemblerCommand> CGameboyArchitecture::parseOpcode(Parser& parser)
{
	GameboyParser gameboyParser;

	return gameboyParser.parseOpcode(parser);
}

void CGameboyArchitecture::NextSection()
{
}

void CGameboyArchitecture::Pass2()
{
}

void CGameboyArchitecture::Revalidate()
{
}
