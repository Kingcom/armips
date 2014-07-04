#pragma once

typedef struct
{
	const char* name;
	const char* mask;
	unsigned char Opcodes[2];
	unsigned char OpcodeLen;
	unsigned char TotalLen;
	int flags;
} tZ80Opcode;


#define Z80_ADD_IMMEDIATE_TIMES_8	0x00000001
#define Z80_8BIT_REGISTER_NIBBLE	0x00000002
#define Z80_8BIT_REGISTER_TIMES_8	0x00000004
#define Z80_IMMEDIATE				0x00000008
#define Z80_16BIT_REGISTER_TIMES_16	0x00000010

extern const tZ80Opcode Z80Opcodes[];
