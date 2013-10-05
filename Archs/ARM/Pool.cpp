#include "stdafx.h"
#include "Pool.h"
#include "Arm.h"
#include "Core/Common.h"

CArmStateCommand::CArmStateCommand(bool state)
{
	armstate = state;
}

bool CArmStateCommand::Validate()
{
	RamPos = Global.RamPos;
	return false;
}

void CArmStateCommand::Encode()
{
	if (Global.SymData.Write == true)
	{
		if (armstate == true)
		{
			fprintf(Global.SymData.Handle,"%08X .arm\n",RamPos);
		} else {
			fprintf(Global.SymData.Handle,"%08X .thumb\n",RamPos);
		}
	}
}

CPool::CPool()
{
	EntryCount = 0;
}

void CPool::Clear()
{
	EntryCount = 0;
}

int CPool::AddEntry(int value)
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

CPoolCommand::CPoolCommand()
{
	RamPos = Global.RamPos;
	PoolId = Arm.NewPool();
	Size = 0;
}

bool CPoolCommand::Validate()
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

void CPoolCommand::Encode()
{
	for (int i = 0; i < Size; i++)
	{
		int num = Arm.GetPool(PoolId).GetEntry(i);
		Global.Output.Write(&num,4);
	}
	Global.RamPos += Size*4;

	if (Global.SymData.Write == true)
	{
		fprintf(Global.SymData.Handle,"%08X .pool\n",RamPos);
		fprintf(Global.SymData.Handle,"%08X .dbl:%04X\n",RamPos,Size*4);
	}
}

void CPoolCommand::WriteTempData(FILE *&Output)
{
	char str[32];

	for (int i = 0; i < Arm.GetPool(PoolId).GetCount(); i++)
	{
		sprintf(str,".word 0x%08X",Arm.GetPool(PoolId).GetEntry(i));
		WriteToTempData(Output,str,RamPos+i*4);
	}
}
