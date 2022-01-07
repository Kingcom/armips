#include "Archs/ARM/Pool.h"

#include "Archs/ARM/Arm.h"
#include "Core/Allocations.h"
#include "Core/Common.h"
#include "Core/FileManager.h"
#include "Core/Misc.h"
#include "Core/SymbolData.h"

#include <unordered_map>

ArmStateCommand::ArmStateCommand(bool state)
{
	armstate = state;
}

bool ArmStateCommand::Validate(const ValidateState &state)
{
	RamPos = g_fileManager->getVirtualAddress();
	return false;
}

void ArmStateCommand::writeSymData(SymbolData& symData) const
{
	// TODO: find a less ugly way to check for undefined memory positions
	if (RamPos == -1)
		return;

	if (armstate)
	{
		symData.addLabel(RamPos, ".arm");
	} else {
		symData.addLabel(RamPos, ".thumb");
	}
}


ArmPoolCommand::ArmPoolCommand()
{
	position = -1;
}

bool ArmPoolCommand::Validate(const ValidateState &state)
{
	int64_t fileID = g_fileManager->getOpenFileID();
	if (position != -1)
		Allocations::forgetPool(fileID, position, values.size() * 4);
	position = g_fileManager->getVirtualAddress();

	size_t oldSize = values.size();
	values.clear();

	std::unordered_map<int32_t, size_t> usedValues;
	for (ArmPoolEntry& entry: Arm.getPoolContent())
	{
		size_t index = values.size();
		
		// try to filter redundant values, but only if
		// we aren't in an unordinarily long validation loop
		if (state.passes < 10)
		{
			auto it = usedValues.find(entry.value);
			if (it != usedValues.end())
				index = it->second;
		}

		if (index == values.size())
		{
			usedValues[entry.value] = index;
			values.push_back(entry.value);
		}

		entry.command->applyFileInfo();
		entry.command->setPoolAddress(position+index*4);
	}

	Arm.clearPoolContent();
	g_fileManager->advanceMemory(values.size()*4);
	Allocations::setPool(fileID, position, values.size() * 4);

	return oldSize != values.size();
}

void ArmPoolCommand::Encode() const
{
	for (size_t i = 0; i < values.size(); i++)
	{
		int32_t value = values[i];
		g_fileManager->writeU32(value);
	}
}

void ArmPoolCommand::writeTempData(TempData& tempData) const
{
	for (size_t i = 0; i < values.size(); i++)
	{
		int32_t value = values[i];
		tempData.writeLine(position+i*4,tfm::format(".word 0x%08X",value));
	}
}

void ArmPoolCommand::writeSymData(SymbolData& symData) const
{
	if (values.size() != 0)
	{
		symData.addLabel(position, ".pool");
		symData.addData(position,values.size()*4,SymbolData::Data32);
	}
}
