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
