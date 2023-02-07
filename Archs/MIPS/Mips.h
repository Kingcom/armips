#pragma once

#include "Archs/Architecture.h"

class Expression;

enum MipsArchType { MARCH_PSX = 0, MARCH_N64, MARCH_PS2, MARCH_PSP, MARCH_RSP, MARCH_INVALID };

class CMipsArchitecture: public Architecture
{
public:
	CMipsArchitecture();
	virtual std::unique_ptr<CAssemblerCommand> parseDirective(Parser& parser);
	virtual std::unique_ptr<CAssemblerCommand> parseOpcode(Parser& parser);
	virtual void registerExpressionFunctions(ExpressionFunctionHandler &handler);
	virtual void NextSection();
	virtual void Pass2() { return; };
	virtual void Revalidate();
	virtual std::unique_ptr<IElfRelocator> getElfRelocator();
	virtual Endianness getEndianness()
	{
		return Version == MARCH_N64 || Version == MARCH_RSP ? Endianness::Big : Endianness::Little;
	};
	virtual int getWordSize() { return 4; };
	void SetLoadDelay(bool Delay, int Register);
	bool GetLoadDelay() { return LoadDelay; };
	int GetLoadDelayRegister() { return LoadDelayRegister; };
	bool GetIgnoreDelay() { return IgnoreLoadDelay; };
	void SetIgnoreDelay(bool b) { IgnoreLoadDelay = b; };
	void SetFixLoadDelay(bool b) { FixLoadDelay = b; };
	bool GetFixLoadDelay() { return FixLoadDelay; };
	void SetVersion(MipsArchType v) { Version = v; };
	MipsArchType GetVersion() { return Version; };
	bool GetDelaySlot() { return DelaySlot; };
	void SetDelaySlot(bool b) {DelaySlot = b; };
	bool hasLoadDelay() { return Version == MARCH_PSX; };
private:
	bool FixLoadDelay;
	bool IgnoreLoadDelay;
	bool LoadDelay;
	int LoadDelayRegister;
	bool DelaySlot;
	MipsArchType Version;
};

typedef struct {
	const char* name;
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

bool MipsGetRegister(const char* source, int& RetLen, MipsRegisterInfo& Result);
int MipsGetRegister(const char* source, int& RetLen);
bool MipsGetFloatRegister(const char* source, int& RetLen, MipsRegisterInfo& Result);
bool MipsGetPs2VectorRegister(const char* source, int& RetLen, MipsRegisterInfo& Result);
int MipsGetFloatRegister(const char* source, int& RetLen);
bool MipsCheckImmediate(const char* Source, Expression& Dest, int& RetLen);

