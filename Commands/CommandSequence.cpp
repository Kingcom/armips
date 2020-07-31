#include "Commands/CommandSequence.h"

CommandSequence::CommandSequence()
	: CAssemblerCommand()
{

}

bool CommandSequence::Validate(const ValidateState &state)
{
	bool result = false;
	
	for (const std::unique_ptr<CAssemblerCommand>& cmd: commands)
	{
		cmd->applyFileInfo();
		if (cmd->Validate(state))
			result = true;
	}

	return result;
}

void CommandSequence::Encode() const
{
	for (const std::unique_ptr<CAssemblerCommand>& cmd: commands)
	{
		cmd->Encode();
	}
}

void CommandSequence::writeTempData(TempData& tempData) const
{
	for (const std::unique_ptr<CAssemblerCommand>& cmd: commands)
	{
		cmd->applyFileInfo();
		cmd->writeTempData(tempData);
	}
}

void CommandSequence::writeSymData(SymbolData& symData) const
{
	for (const std::unique_ptr<CAssemblerCommand>& cmd: commands)
	{
		cmd->writeSymData(symData);
	}
}
