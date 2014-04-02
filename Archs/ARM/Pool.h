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
	int RamPos;
	bool armstate;
};


class ArmPool
{
public:
	ArmPool();
	void Clear();
	int AddEntry(int value);
	int GetCount() { return EntryCount; };
	int GetEntry(int num) { return Entries[num]; };
	void SetRamPos(int num) { RamPos = num; };  
	int GetRamPos() { return RamPos; };
private:
	int Entries[512];
	int EntryCount;
	int RamPos;
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
	int RamPos;
	int PoolId;
	int Size;
};
