#pragma once

#include "Commands/CAssemblerCommand.h"

#include <cstdint>
#include <vector>

class ArmStateCommand: public CAssemblerCommand
{
public:
	ArmStateCommand(bool state);
	bool Validate(const ValidateState &state) override;
	virtual void Encode() const { };
	virtual void writeTempData(TempData& tempData) const { };
	virtual void writeSymData(SymbolData& symData) const;
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
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
	virtual void writeSymData(SymbolData& symData) const;
private:
	int64_t position;
	std::vector<int32_t> values;
};
