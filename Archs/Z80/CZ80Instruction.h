#pragma once

#include "Archs/Z80/Z80.h"
#include "Archs/Z80/Z80Opcodes.h"
#include "Commands/CAssemblerCommand.h"

struct Z80OpcodeVariables
{
	Z80RegisterValue LeftParam;
	Z80RegisterValue RightParam;
	Expression ImmediateExpression;
	int64_t Immediate;
	unsigned char Length;
	unsigned char Encoding;
	bool IsNegative : 1;
	bool WriteImmediate8 : 1;
	bool WriteImmediate16 : 1;
};

class CZ80Instruction: public CAssemblerCommand
{
public:
	CZ80Instruction(const tZ80Opcode& sourceOpcode, Z80OpcodeVariables& vars);
	bool Validate(const ValidateState& state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
private:
	Z80OpcodeVariables Vars;
	tZ80Opcode Opcode;
	int64_t RamPos;
};
