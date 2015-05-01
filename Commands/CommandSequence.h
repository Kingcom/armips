#pragma once
#include "Commands/CAssemblerCommand.h"

class Label;

class CommandSequence: public CAssemblerCommand
{
public:
	CommandSequence();
	virtual ~CommandSequence();
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
	virtual void writeSymData(SymbolData& symData);
	void addCommand(CAssemblerCommand* cmd) { commands.push_back(cmd); }
private:
	std::vector<CAssemblerCommand*> commands;
};