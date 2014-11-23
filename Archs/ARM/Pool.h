#pragma once
#include "Commands/CAssemblerCommand.h"

class ArmStateCommand: public CAssemblerCommand
{
public:
	ArmStateCommand(bool state);
	virtual bool Validate();
	virtual void Encode() { };
	virtual void writeTempData(TempData& tempData) { };
	virtual void writeSymData(SymbolData& symData);
private:
	u64 RamPos;
	bool armstate;
};


class ArmPool
{
public:
	ArmPool();
	void Clear();
	u64 AddEntry(u32 value);
	size_t GetCount() { return EntryCount; };
	u32 GetEntry(size_t num) { return Entries[num]; };
	void SetRamPos(u64 num) { RamPos = num; };  
	u64 GetRamPos() { return RamPos; };
private:
	u32 Entries[512];
	size_t EntryCount;
	u64 RamPos;
};

class ArmPoolCommand: public CAssemblerCommand
{
public:
	ArmPoolCommand();
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
	virtual void writeSymData(SymbolData& symData);
	virtual bool IsPool() { return true; };
private:
	u64 RamPos;
	size_t PoolId;
	size_t Size;
};
