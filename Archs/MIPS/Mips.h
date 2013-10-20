#pragma once
#include "Util/CommonClasses.h"
#include "Core/MathParser.h"

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
	virtual void AssembleOpcode(const std::wstring& name, const std::wstring& args);
	virtual bool AssembleDirective(const std::wstring& name, const std::wstring& args);
	virtual void NextSection();
	virtual void Pass2() { return; };
	virtual void Revalidate() { return; };
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
	bool hasLoadDelay() { return (Version & MARCH_PSX) != 0; };
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
	char name[5];
	short num;
} MipsRegisterInfo;

enum MipsVfpuType { MIPSVFPU_VECTOR, MIPSVFPU_MATRIX };

struct MipsVFPURegister
{
	MipsVfpuType type;
	int num;
	char name[8];
};

extern const tMipsRegister MipsRegister[];
extern CMipsArchitecture Mips;

bool MipsGetRegister(char* source, int& RetLen, MipsRegisterInfo& Result);
int MipsGetRegister(char* source, int& RetLen);
bool MipsGetFloatRegister(char* source, int& RetLen, MipsRegisterInfo& Result);
int MipsGetFloatRegister(char* source, int& RetLen);
bool MipsCheckImmediate(char* Source, MathExpression& Dest, int& RetLen);

