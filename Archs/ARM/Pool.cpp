#include "stdafx.h"
#include "Pool.h"
#include "Arm.h"
#include "Core/Common.h"
#include "Core/FileManager.h"

ArmStateCommand::ArmStateCommand(bool state)
{
	armstate = state;
}

bool ArmStateCommand::Validate()
{
	RamPos = g_fileManager->getVirtualAddress();
	return false;
}

void ArmStateCommand::writeSymData(SymbolData& symData)
{
	if (armstate == true)
	{
		symData.addLabel(RamPos,L".arm");
	} else {
		symData.addLabel(RamPos,L".thumb");
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

u64 ArmPool::AddEntry(u32 value)
{
	if (Global.validationPasses < 10)
	{
		for (size_t i = 0; i < EntryCount; i++)
		{
			if (Entries[i] == value)
				return RamPos+i*4;
		}
	}

	if (EntryCount == 512) return -1;
	Entries[EntryCount] = value;
	return RamPos+(EntryCount++*4);
}

ArmPoolCommand::ArmPoolCommand()
{
	RamPos = g_fileManager->getVirtualAddress();
	PoolId = Arm.NewPool();
	Size = 0;
}

bool ArmPoolCommand::Validate()
{
	bool Result = false;
	ArmPool& Pool = Arm.GetPool(PoolId);

	if (Pool.GetRamPos() != g_fileManager->getVirtualAddress())
	{
		RamPos = g_fileManager->getVirtualAddress();
		Pool.SetRamPos(RamPos);
		Result = true;
	}
	if (Pool.GetCount() != Size)
	{
		Size = Pool.GetCount();
		Result = true;
	}

	g_fileManager->advanceMemory(Size*4);
	Arm.NextPool();
	return Result;
}

void ArmPoolCommand::Encode()
{
	for (size_t i = 0; i < Size; i++)
	{
		int num = Arm.GetPool(PoolId).GetEntry(i);
		g_fileManager->write(&num,4);
	}
}

void ArmPoolCommand::writeTempData(TempData& tempData)
{
	for (size_t i = 0; i < Arm.GetPool(PoolId).GetCount(); i++)
	{
		tempData.writeLine(RamPos+i*4,formatString(L".word 0x%08X",Arm.GetPool(PoolId).GetEntry(i)));
	}
}

void ArmPoolCommand::writeSymData(SymbolData& symData)
{
	if (Size > 0)
	{
		symData.addLabel(RamPos,L".pool");
		symData.addData(RamPos,Size*4,SymbolData::Data32);
	}
}
