#pragma once

#include "Parser/Parser.h"

struct Z80RegisterDescriptor {
	const wchar_t* name;
	int num;
};

class Z80Parser
{
public:
	std::unique_ptr<CAssemblerCommand> parseDirective(Parser& parser);
	std::unique_ptr<CZ80Instruction> parseOpcode(Parser& parser);
private:
	bool parseRegisterTable(Parser& parser, Z80RegisterValue& dest, const Z80RegisterDescriptor* table, size_t count, int allowed);
	bool parseRegister8(Parser& parser, Z80RegisterValue& dest, int allowed);
	bool parseRegister16SP(Parser& parser, Z80RegisterValue& dest, int allowed);
	bool parseRegister16AF(Parser& parser, Z80RegisterValue& dest, int allowed);
	bool parseCondition(Parser& parser, Z80RegisterValue& dest);
	bool parseHLIncDec(Parser& parser, Z80RegisterValue& dest);
	bool parseMemoryRegister16(Parser& parser, Z80RegisterValue& dest, int allowed);
	bool parseMemoryImmediate(Parser& parser, Expression& dest);
	bool parseFF00PlusC(Parser& parser);
	bool parseSPImmediate(Parser& parser, Expression& dest, bool& isNegative);
	bool parseOpcodeParameter(Parser& parser, unsigned char paramType, Z80RegisterValue& destReg, Expression& destImm, bool& isNegative);
	bool parseOpcodeParameterList(Parser& parser, const tZ80Opcode, Z80OpcodeVariables& vars);
};
