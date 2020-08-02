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
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
	void addCommand(std::unique_ptr<CAssemblerCommand> cmd) { commands.push_back(std::move(cmd)); }
private:
	std::vector<std::unique_ptr<CAssemblerCommand>> commands;
};
