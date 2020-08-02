#pragma once

#include "Commands/CAssemblerCommand.h"

#include <cstdint>
#include <vector>

class ArmStateCommand: public CAssemblerCommand
{
public:
	ArmStateCommand(bool state);
	bool Validate(const ValidateState &state) override;
	void Encode() const override { };
	void writeTempData(TempData& tempData) const override { };
	void writeSymData(SymbolData& symData) const override;
private:
	int64_t RamPos;
	bool armstate;
};

class ArmOpcodeCommand;

struct ArmPoolEntry
{
	ArmOpcodeCommand* command;
	int32_t value;
};

class ArmPoolCommand: public CAssemblerCommand
{
public:
	ArmPoolCommand();
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
private:
	int64_t position;
	std::vector<int32_t> values;
};
