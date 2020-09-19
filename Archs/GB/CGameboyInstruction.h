#pragma once

#include "Archs/GB/Gameboy.h"
#include "Archs/GB/GameboyOpcodes.h"
#include "Commands/CAssemblerCommand.h"

struct GameboyOpcodeVariables
{
	GameboyRegisterValue LeftParam;
	GameboyRegisterValue RightParam;
	Expression ImmediateExpression;
	int64_t Immediate;
	unsigned char Length;
	unsigned char Encoding;
	bool IsNegative : 1;
	bool WritePrefix : 1;
	bool WriteImmediate8 : 1;
	bool WriteImmediate16 : 1;
};

class CGameboyInstruction: public CAssemblerCommand
{
public:
	CGameboyInstruction(const tGameboyOpcode& sourceOpcode, GameboyOpcodeVariables& vars);
	bool Validate(const ValidateState& state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
private:
	GameboyOpcodeVariables Vars;
	tGameboyOpcode Opcode;

	// Inherited via CAssemblerCommand
};
