#pragma once
#include "Util/CommonClasses.h"

#define MARCH_PSX			0x0000001
#define MARCH_N64			0x0000002
#define MARCH_PS2			0x0000004
#define MARCH_PSP			0x0000008

#define MARCH_LV2			MARCH_PS2|MARCH_PSP
#define MARCH_ALL			MARCH_PSX|MARCH_PS2|MARCH_PSP

class CMipsArchitecture: public CArchitecture
{
public:
	CMipsArchitecture();
	virtual void AssembleOpcode(char* name, char* args);
	virtual bool AssembleDirective(char* name, char* args);
	virtual void NextSection();
	virtual void Pass2() { return; };
	virtual void Revalidate() { return; };
	virtual int GetFileAddress(int MemoryAddress) { return 0; };
	virtual int GetMemoryAddress(int FileAddress) { return 0; };
	virtual int GetWordSize();
	void SetLoadDelay(bool Delay, int Register);
	bool GetLoadDelay() { return LoadDelay; };
	int GetLoadDelayRegister() { return LoadDelayRegister; };
	bool GetIgnoreDelay() { return IgnoreLoadDelay; };
	void SetIgnoreDelay(bool b) { IgnoreLoadDelay = b; };
	void SetFixLoadDelay(bool b) { FixLoadDelay = b; };
	bool GetFixLoadDelay() { return FixLoadDelay; };
	void SetVersion(int v) { Version = v; };
	int GetVersion() { return Version; };
	bool GetDelaySlot() { return DelaySlot; };
	void SetDelaySlot(bool b) {DelaySlot = b; };
private:
	bool FixLoadDelay;
	bool IgnoreLoadDelay;
	bool LoadDelay;
	int LoadDelayRegister;
	bool DelaySlot;
	int Version;
};

typedef struct {
	char* name;
	short num;
	short len;
} tMipsRegister;

typedef struct {
	char Name[5];
	short Number;
} tMipsRegisterInfo;

extern const tMipsRegister MipsRegister[];
extern CMipsArchitecture Mips;

bool MipsGetRegister(char* source, int& RetLen, tMipsRegisterInfo& Result);
int MipsGetRegister(char* source, int& RetLen);
bool MipsGetFloatRegister(char* source, int& RetLen, tMipsRegisterInfo& Result);
int MipsGetFloatRegister(char* source, int& RetLen);
bool MipsCheckImmediate(char* Source, char* Dest, int& RetLen, CStringList& List);

