#pragma once

#include "Parser/Parser.h"

struct GameboyRegisterDescriptor {
	const wchar_t* name;
	int num;
};

class GameboyParser
{
public:
	std::unique_ptr<CAssemblerCommand> parseDirective(Parser& parser);
	std::unique_ptr<CGameboyInstruction> parseOpcode(Parser& parser);
private:
	bool parseRegisterTable(Parser& parser, GameboyRegisterValue& dest, const GameboyRegisterDescriptor* table, size_t count, int allowed);
	bool parseRegister8(Parser& parser, GameboyRegisterValue& dest, int allowed);
	bool parseRegister16SP(Parser& parser, GameboyRegisterValue& dest, int allowed);
	bool parseRegister16AF(Parser& parser, GameboyRegisterValue& dest, int allowed);
	bool parseHLIncDec(Parser& parser, GameboyRegisterValue& dest);
	bool parseMemoryRegister16(Parser& parser, GameboyRegisterValue& dest, int allowed);
	bool parseMemoryImmediate(Parser& parser, Expression& dest);
	bool parseFF00PlusC(Parser& parser);
	bool parseSPImmediate(Parser& parser, Expression& dest, bool& isNegative);
	bool parseOpcodeParameter(Parser& parser, unsigned char paramType, GameboyRegisterValue& destReg, Expression& destImm, bool& isNegative);
	bool parseOpcodeParameterList(Parser& parser, const tGameboyOpcode, GameboyOpcodeVariables& vars);
};
