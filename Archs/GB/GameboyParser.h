#pragma once

#include "Parser/Parser.h"

struct GameboyParameterDescriptor {
	const wchar_t* name;
	int num;
};

class GameboyParser
{
public:
	std::unique_ptr<CAssemblerCommand> parseDirective(Parser& parser);
	std::unique_ptr<CGameboyInstruction> parseOpcode(Parser& parser);
private:
	bool parseRegisterTable(Parser& parser, GameboyRegisterValue& dest, const GameboyParameterDescriptor* table, size_t count, int allowed);
	bool parseRegister8(Parser& parser, GameboyRegisterValue& dest, int allowed);
	bool parseRegister16(Parser& parser, GameboyRegisterValue& dest, int allowed);
	bool parseMemoryRegister16(Parser& parser, GameboyRegisterValue& dest, int allowed);
	bool parseMemoryImmediate(Parser& parser, Expression& dest);
	bool parseOpcodeParameter(Parser& parser, unsigned char paramType, GameboyRegisterValue& destReg, Expression& destImm);
	bool parseOpcodeParameterList(Parser& parser, const tGameboyOpcode, GameboyOpcodeVariables& vars);
};
