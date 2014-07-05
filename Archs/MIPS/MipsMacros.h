#pragma once
#include "Core/MathParser.h"
#include "Mips.h"
#include "CMipsInstruction.h"

#define MIPSM_B						0x00000001
#define MIPSM_BU					0x00000002
#define MIPSM_HW					0x00000004
#define MIPSM_HWU					0x00000008
#define MIPSM_W						0x00000010
#define MIPSM_NE					0x00000020
#define MIPSM_LT					0x00000040
#define MIPSM_GE					0x00000080
#define MIPSM_EQ					0x00000100
#define MIPSM_IMM					0x00000200
#define MIPSM_LEFT					0x00000400
#define MIPSM_RIGHT					0x00000800
#define MIPSM_UNALIGNED				0x00001000
#define MIPSM_DONTWARNDELAYSLOT		0x00002000
#define MIPSM_UPPER					0x00004000
#define MIPSM_LOWER					0x00008000

typedef struct {
	MathExpression i1;
	MathExpression i2;
	MipsRegisterInfo rs;			// source reg
	MipsRegisterInfo rt;			// target reg
	MipsRegisterInfo rd;			// dest reg
	bool NoCheckError;
} tMipsMacroVars;

typedef struct {
	MathExpression i1;
	MathExpression i2;
	MipsRegisterInfo rs;			// source reg
	MipsRegisterInfo rt;			// target reg
	MipsRegisterInfo rd;			// dest reg
} tMipsMacroData;

typedef struct {
	int i1;
	int i2;
	MipsRegisterInfo rs;			// source reg
	MipsRegisterInfo rt;			// target reg
	MipsRegisterInfo rd;			// dest reg
} tMipsMacroValues;

typedef int (*mipsmacrofunc)(tMipsMacroValues&,int,CMipsInstruction*);

typedef struct {
	const char* name;
	const char* args;
	int MaxOpcodes;
	mipsmacrofunc Function;
	int flags;
} tMipsMacro;

extern const tMipsMacro MipsMacros[];


bool MipsCheckMacro(char* Opcode, char* Arguments);

