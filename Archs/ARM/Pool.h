#pragma once
#include "Commands/CAssemblerCommand.h"

class CArmStateCommand: public CAssemblerCommand
{
public:
	CArmStateCommand(bool state);
	virtual bool Validate();
	virtual void Encode();
	virtual void WriteTempData(FILE*& Output) { return; };
private:
	int RamPos;
	bool armstate;
};


class CPool
{
public:
	CPool();
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

class CPoolCommand: public CAssemblerCommand
{
public:
	CPoolCommand();
	virtual bool Validate();
	virtual void Encode();
	virtual void WriteTempData(FILE*& Output);
private:
	int RamPos;
	int PoolId;
	int Size;
};
