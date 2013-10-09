#include "stdafx.h"
#include "Pool.h"
#include "Arm.h"
#include "Core/Common.h"

ArmStateCommand::ArmStateCommand(bool state)
{
	armstate = state;
}

bool ArmStateCommand::Validate()
{
	RamPos = Global.RamPos;
	return false;
}

void ArmStateCommand::writeSymData(SymbolData& symData)
{
	if (armstate == true)
	{
		symData.addSymbol(RamPos,L".arm");
	} else {
		symData.addSymbol(RamPos,L".thumb");
	}
}

ArmPool::ArmPool()
{
	EntryCount = 0;
}

void ArmPool::Clear()
{
	EntryCount = 0;
}

int ArmPool::AddEntry(int value)
{
	if (Global.validationPasses < 10)
	{
		for (int i = 0; i < EntryCount; i++)
		{
			if (Entries[i] == value) return RamPos+i*4;
		}
	}

	if (EntryCount == 512) return -1;
	Entries[EntryCount] = value;
	return RamPos+(EntryCount++*4);
}

ArmPoolCommand::ArmPoolCommand()
{
	RamPos = Global.RamPos;
	PoolId = Arm.NewPool();
	Size = 0;
}

bool ArmPoolCommand::Validate()
{
	bool Result = false;
	if (Arm.GetPool(PoolId).GetRamPos() != Global.RamPos)
	{
		RamPos = Global.RamPos;
		Arm.GetPool(PoolId).SetRamPos(RamPos);
		Result = true;
	}
	if (Arm.GetPool(PoolId).GetCount() != Size)
	{
		Size = Arm.GetPool(PoolId).GetCount();
		Result = true;
	}

	Global.RamPos += Size*4;
	Arm.NextPool();
	return Result;
}

void ArmPoolCommand::Encode()
{
	for (int i = 0; i < Size; i++)
	{
		int num = Arm.GetPool(PoolId).GetEntry(i);
		Global.Output.write(&num,4);
	}
	Global.RamPos += Size*4;
}

void ArmPoolCommand::writeTempData(TempData& tempData)
{
	for (int i = 0; i < Arm.GetPool(PoolId).GetCount(); i++)
	{
		tempData.writeLine(RamPos+i*4,formatString(L".word 0x%08X",Arm.GetPool(PoolId).GetEntry(i)));
	}
}

void ArmPoolCommand::writeSymData(SymbolData& symData)
{
	symData.addSymbol(RamPos,L".pool");
	symData.addSymbol(RamPos,formatString(L".dbl:%04X",Size*4));
}
