#pragma once
#include "Util/CommonClasses.h"
#include "Core/MathParser.h"

#define MARCH_PSX			0x0000001
#define MARCH_N64			0x0000002
#define MARCH_PS2			0x0000004
#define MARCH_PSP			0x0000008

#define MARCH_LV2			MARCH_PS2|MARCH_PSP
#define MARCH_ALL			MARCH_PSX|MARCH_PS2|MARCH_PSP

enum {
	R_MIPS_NONE,
	R_MIPS_16,
	R_MIPS_32,
	R_MIPS_REL32,
	R_MIPS_26,
	R_MIPS_HI16,
	R_MIPS_LO16,
	R_MIPS_GPREL16,
	R_MIPS_LITERAL,
	R_MIPS_GOT16,
	R_MIPS_PC16,
	R_MIPS_CALL16,
	R_MIPS_GPREL32
};

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
	virtual IElfRelocator* getElfRelocator();
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
bool MipsGetVFPURegister(char* line, MipsVFPURegister& reg, int size);
