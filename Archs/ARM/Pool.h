#pragma once
#include "Commands/CAssemblerCommand.h"
#include "../Architecture.h"

class ArmStateCommand: public CAssemblerCommand
{
public:
	ArmStateCommand(bool state);
	virtual bool Validate();
	virtual void Encode() const { };
	virtual void writeTempData(TempData& tempData) const { };
	virtual void writeSymData(SymbolData& symData) const;
private:
	u64 RamPos;
	bool armstate;
};

class ArmOpcodeCommand;

struct ArmPoolEntry
{
	ArmOpcodeCommand* command;
	u32 value;
};

class ArmPoolCommand: public CAssemblerCommand
{
public:
	ArmPoolCommand();
	virtual bool Validate();
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
	virtual void writeSymData(SymbolData& symData) const;
private:
	u64 position;
	std::vector<u32> values;
};
