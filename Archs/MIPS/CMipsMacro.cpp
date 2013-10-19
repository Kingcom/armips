#include "stdafx.h"
#include "CMipsMacro.h"
#include "MipsMacros.h"
#include "CMipsInstruction.h"
#include "Core/Common.h"
#include "Mips.h"
#include "MipsOpcodes.h"
#include "Core/FileManager.h"

CMipsMacro::CMipsMacro(int num, tMipsMacroVars& InputVars)
{
	InstructionAmount = MipsMacros[num].MaxOpcodes;
	Instructions = new CMipsInstruction[InstructionAmount];
	MacroNum = num;

	Data.i1 = InputVars.i1;
	Data.i2 = InputVars.i2;
	Data.rd = InputVars.rd;
	Data.rs = InputVars.rs;
	Data.rt = InputVars.rt;
	IgnoreLoadDelay = Mips.GetIgnoreDelay();
	
	int SpaceNeeded = InstructionAmount*4;
	g_fileManager->advanceMemory(SpaceNeeded);
}

CMipsMacro::~CMipsMacro()
{
	delete[] Instructions;
}


bool CMipsMacro::Validate()
{
	tMipsMacroValues Values;
	CStringList List;

	Values.rd = Data.rd;
	Values.rs = Data.rs;
	Values.rt = Data.rt;

	if (Data.i1.isLoaded() && Data.i1.evaluate(Values.i1,true) == false)
		return false;
	
	if (Data.i2.isLoaded() && Data.i2.evaluate(Values.i2,true) == false)
		return false;

	int NewNum = MipsMacros[MacroNum].Function(Values,MipsMacros[MacroNum].flags,Instructions);

	if (IgnoreLoadDelay == false && Mips.GetDelaySlot() == true && NewNum > 1
		&& (MipsMacros[MacroNum].flags & MIPSM_DONTWARNDELAYSLOT) == 0)
	{
		Logger::queueError(Logger::Warning,L"Macro with multiple opcodes used inside a delay slot");
	}

	for (int i = 0; i < NewNum; i++)
	{
		Instructions[i].Validate();
	}

	if (NewNum != InstructionAmount)	// amount changed
	{
		InstructionAmount = NewNum;
		return true;
	} else {
		return false;
	}
}

void CMipsMacro::Encode()
{
	for (int i = 0; i < InstructionAmount; i++)
	{
		Instructions[i].Encode();
	}
}

void CMipsMacro::writeTempData(TempData& tempData)
{
	for (int i = 0; i < InstructionAmount; i++)
	{
		Instructions[i].writeTempData(tempData);
	}
}