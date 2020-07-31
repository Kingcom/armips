#pragma once

#include "Commands/CAssemblerCommand.h"

#include <memory>
#include <vector>

class Label;

class CommandSequence: public CAssemblerCommand
{
public:
	CommandSequence();
	bool Validate(const ValidateState &state) override;
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
	virtual void writeSymData(SymbolData& symData) const;
	void addCommand(std::unique_ptr<CAssemblerCommand> cmd) { commands.push_back(std::move(cmd)); }
private:
	std::vector<std::unique_ptr<CAssemblerCommand>> commands;
};
