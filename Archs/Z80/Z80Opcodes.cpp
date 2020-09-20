#include "Archs/Z80/Z80Opcodes.h"

// Order:
// - Everything else
// - SP_IMM and MEMIX_MEMIY
// - MEMIMMEDIATE
// - IMMEDIATE
const tZ80Opcode Z80Opcodes[] = {
	//	Name	Len	Encode	Left param				Right param				LShift	RShift	Flags
	{ L"nop",	1,	0x00,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		0 },
	{ L"ld",	1,	0x40,	Z80_PARAM_REG8_MEMHL,	Z80_PARAM_REG8_MEMHL,	3,		0,		Z80_LOAD_REG8_REG8 },
	{ L"ld",	1,	0x02,	Z80_PARAM_MEMBC_MEMDE,	Z80_PARAM_A,			4,		-1,		0 },
	{ L"ld",	1,	0x0A,	Z80_PARAM_A,			Z80_PARAM_MEMBC_MEMDE,	-1,		4,		0 },
	{ L"ld",	1,	0x22,	Z80_PARAM_HLI_HLD,		Z80_PARAM_A,			4,		-1,		Z80_GAMEBOY },
	{ L"ld",	1,	0x2A,	Z80_PARAM_A,			Z80_PARAM_HLI_HLD,		-1,		4,		Z80_GAMEBOY },
	{ L"ld",	1,	0xE2,	Z80_PARAM_FF00_C,		Z80_PARAM_A,			-1,		-1,		Z80_GAMEBOY },
	{ L"ld",	2,	0x57,	Z80_PARAM_A,			Z80_PARAM_IR,			-1,		3,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"ld",	1,	0xF2,	Z80_PARAM_A,			Z80_PARAM_FF00_C,		-1,		-1,		Z80_GAMEBOY },
	{ L"ld",	1,	0xF9,	Z80_PARAM_SP,			Z80_PARAM_HL,			-1,		-1,		0 },
	{ L"ld",	2,	0xF8,	Z80_PARAM_HL,			Z80_PARAM_SP_IMM,		-1,		-1,		Z80_IMMEDIATE_S8 | Z80_GAMEBOY },
	{ L"ld",	2,	0xF9,	Z80_PARAM_REG16_SP,		Z80_PARAM_IX_IY,		-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY },
	{ L"ld",	2,	0x47,	Z80_PARAM_IR,			Z80_PARAM_A,			3,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"ld",	3,	0x46,	Z80_PARAM_REG8,			Z80_PARAM_MEMIX_MEMIY,	3,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"ld",	3,	0x2A,	Z80_PARAM_HL,			Z80_PARAM_MEMIMMEDIATE,	-1,		-1,		Z80_IMMEDIATE_U16 | Z80_Z80 },
	{ L"ld",	4,	0x4B,	Z80_PARAM_REG16_SP,		Z80_PARAM_MEMIMMEDIATE,	4,		-1,		Z80_IMMEDIATE_U16 | Z80_Z80 | Z80_PREFIX_ED },
	{ L"ld",	3,	0x3A,	Z80_PARAM_A,			Z80_PARAM_MEMIMMEDIATE,	-1,		-1,		Z80_IMMEDIATE_U16 | Z80_Z80 },
	{ L"ld",	3,	0xFA,	Z80_PARAM_A,			Z80_PARAM_MEMIMMEDIATE,	-1,		-1,		Z80_IMMEDIATE_U16 | Z80_GAMEBOY },
	{ L"ld",	4,	0x2A,	Z80_PARAM_IX_IY,		Z80_PARAM_MEMIMMEDIATE,	-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U16 },
	{ L"ld",	2,	0x06,	Z80_PARAM_REG8_MEMHL,	Z80_PARAM_IMMEDIATE,	3,		-1,		Z80_IMMEDIATE_U8 },
	{ L"ld",	3,	0x01,	Z80_PARAM_REG16_SP,		Z80_PARAM_IMMEDIATE,	4,		-1,		Z80_IMMEDIATE_U16 },
	{ L"ld",	4,	0x21,	Z80_PARAM_IX_IY,		Z80_PARAM_IMMEDIATE,	-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U16 },
	{ L"ld",	3,	0x70,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_REG8,			-1,		0,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"ld",	4,	0x36,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_IMMEDIATE,	-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 | Z80_IMMEDIATE2_U8 },
	{ L"ld",	3,	0x22,	Z80_PARAM_MEMIMMEDIATE,	Z80_PARAM_HL,			-1,		-1,		Z80_IMMEDIATE_U16 | Z80_Z80 },
	{ L"ld",	4,	0x43,	Z80_PARAM_MEMIMMEDIATE,	Z80_PARAM_REG16_SP,		-1,		4,		Z80_IMMEDIATE_U16 | Z80_Z80 | Z80_PREFIX_ED },
	{ L"ld",	3,	0x32,	Z80_PARAM_MEMIMMEDIATE,	Z80_PARAM_A,			-1,		-1,		Z80_IMMEDIATE_U16 | Z80_Z80 },
	{ L"ld",	3,	0x08,	Z80_PARAM_MEMIMMEDIATE,	Z80_PARAM_SP,			-1,		-1,		Z80_IMMEDIATE_U16 | Z80_GAMEBOY },
	{ L"ld",	3,	0xEA,	Z80_PARAM_MEMIMMEDIATE,	Z80_PARAM_A,			-1,		-1,		Z80_IMMEDIATE_U16 | Z80_GAMEBOY },
	{ L"ld",	4,	0x22,	Z80_PARAM_MEMIMMEDIATE,	Z80_PARAM_IX_IY,		-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U16 },
	{ L"ldi",	1,	0x22,	Z80_PARAM_MEMHL,		Z80_PARAM_A,			-1,		-1,		Z80_GAMEBOY },
	{ L"ldi",	1,	0x2A,	Z80_PARAM_A,			Z80_PARAM_MEMHL,		-1,		-1,		Z80_GAMEBOY },
	{ L"ldd",	1,	0x32,	Z80_PARAM_MEMHL,		Z80_PARAM_A,			-1,		-1,		Z80_GAMEBOY },
	{ L"ldd",	1,	0x3A,	Z80_PARAM_A,			Z80_PARAM_MEMHL,		-1,		-1,		Z80_GAMEBOY },
	{ L"ldh",	2,	0xE0,	Z80_PARAM_MEMIMMEDIATE,	Z80_PARAM_A,			-1,		-1,		Z80_IMMEDIATE_U8 | Z80_GAMEBOY },
	{ L"ldh",	2,	0xF0,	Z80_PARAM_A,			Z80_PARAM_MEMIMMEDIATE,	-1,		-1,		Z80_IMMEDIATE_U8 | Z80_GAMEBOY },
	{ L"ldhl",	2,	0xF8,	Z80_PARAM_SP,			Z80_PARAM_IMMEDIATE,	-1,		-1,		Z80_IMMEDIATE_S8 | Z80_GAMEBOY },
	{ L"push",	1,	0xC5,	Z80_PARAM_REG16_AF,		Z80_PARAM_NONE,			4,		-1,		0 },
	{ L"push",	2,	0xE5,	Z80_PARAM_IX_IY,		Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY },
	{ L"pop",	1,	0xC1,	Z80_PARAM_REG16_AF,		Z80_PARAM_NONE,			4,		-1,		0 },
	{ L"pop",	2,	0xE1,	Z80_PARAM_IX_IY,		Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY },
	{ L"add",	1,	0x09,	Z80_PARAM_HL,			Z80_PARAM_REG16_SP,		-1,		4,		0 },
	{ L"add",	1,	0x80,	Z80_PARAM_A,			Z80_PARAM_REG8_MEMHL,	-1,		0,		0 },
	{ L"add",	3,	0x86,	Z80_PARAM_A,			Z80_PARAM_MEMIX_MEMIY,	-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"add",	2,	0xC6,	Z80_PARAM_A,			Z80_PARAM_IMMEDIATE,	-1,		-1,		Z80_IMMEDIATE_U8 | Z80_ADD_SUB_IMMEDIATE },
	{ L"add",	2,	0xE8,	Z80_PARAM_SP,			Z80_PARAM_IMMEDIATE,	-1,		-1,		Z80_IMMEDIATE_S8 | Z80_GAMEBOY },
	{ L"add",	2,	0x09,	Z80_PARAM_IX_IY,		Z80_PARAM_REG16_IX_IY,	-1,		4,		Z80_Z80 | Z80_PREFIX_IX_IY },
	{ L"adc",	1,	0x88,	Z80_PARAM_A,			Z80_PARAM_REG8_MEMHL,	-1,		0,		0 },
	{ L"adc",	3,	0x8E,	Z80_PARAM_A,			Z80_PARAM_MEMIX_MEMIY,	-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"adc",	2,	0xCE,	Z80_PARAM_A,			Z80_PARAM_IMMEDIATE,	-1,		-1,		Z80_IMMEDIATE_U8 | Z80_ADD_SUB_IMMEDIATE },
	{ L"adc",	2,	0x4A,	Z80_PARAM_HL,			Z80_PARAM_REG16_SP,		-1,		4,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"sub",	1,	0x90,	Z80_PARAM_A,			Z80_PARAM_REG8_MEMHL,	-1,		0,		0 },
	{ L"sub",	3,	0x96,	Z80_PARAM_A,			Z80_PARAM_MEMIX_MEMIY,	-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"sub",	2,	0xD6,	Z80_PARAM_A,			Z80_PARAM_IMMEDIATE,	-1,		-1,		Z80_IMMEDIATE_U8 | Z80_ADD_SUB_IMMEDIATE },
	{ L"sub",	2,	0xE8,	Z80_PARAM_SP,			Z80_PARAM_IMMEDIATE,	-1,		-1,		Z80_IMMEDIATE_S8 | Z80_NEGATE_IMM | Z80_GAMEBOY },
	{ L"sub",	1,	0x90,	Z80_PARAM_REG8_MEMHL,	Z80_PARAM_NONE,			0,		-1,		0 },
	{ L"sub",	3,	0x96,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"sub",	2,	0xD6,	Z80_PARAM_IMMEDIATE,	Z80_PARAM_NONE,			-1,		-1,		Z80_IMMEDIATE_U8 | Z80_ADD_SUB_IMMEDIATE },
	{ L"sbc",	1,	0x98,	Z80_PARAM_A,			Z80_PARAM_REG8_MEMHL,	-1,		0,		0 },
	{ L"sbc",	3,	0x9E,	Z80_PARAM_A,			Z80_PARAM_MEMIX_MEMIY,	-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"sbc",	2,	0xDE,	Z80_PARAM_A,			Z80_PARAM_IMMEDIATE,	-1,		-1,		Z80_IMMEDIATE_U8 | Z80_ADD_SUB_IMMEDIATE },
	{ L"sbc",	2,	0x42,	Z80_PARAM_HL,			Z80_PARAM_REG16_SP,		-1,		4,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"sbc",	3,	0x9E,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"and",	1,	0xA0,	Z80_PARAM_A,			Z80_PARAM_REG8_MEMHL,	-1,		0,		0 },
	{ L"and",	3,	0xA6,	Z80_PARAM_A,			Z80_PARAM_MEMIX_MEMIY,	-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"and",	2,	0xE6,	Z80_PARAM_A,			Z80_PARAM_IMMEDIATE,	-1,		-1,		Z80_IMMEDIATE_U8 },
	{ L"and",	1,	0xA0,	Z80_PARAM_REG8_MEMHL,	Z80_PARAM_NONE,			0,		-1,		0 },
	{ L"and",	3,	0xA6,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"and",	2,	0xE6,	Z80_PARAM_IMMEDIATE,	Z80_PARAM_NONE,			-1,		-1,		Z80_IMMEDIATE_U8 },
	{ L"xor",	1,	0xA8,	Z80_PARAM_A,			Z80_PARAM_REG8_MEMHL,	-1,		0,		0 },
	{ L"xor",	3,	0xAE,	Z80_PARAM_A,			Z80_PARAM_MEMIX_MEMIY,	-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"xor",	2,	0xEE,	Z80_PARAM_A,			Z80_PARAM_IMMEDIATE,	-1,		-1,		Z80_IMMEDIATE_U8 },
	{ L"xor",	1,	0xA8,	Z80_PARAM_REG8_MEMHL,	Z80_PARAM_NONE,			0,		-1,		0 },
	{ L"xor",	3,	0xAE,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"xor",	2,	0xEE,	Z80_PARAM_IMMEDIATE,	Z80_PARAM_NONE,			-1,		-1,		Z80_IMMEDIATE_U8 },
	{ L"or",	1,	0xB0,	Z80_PARAM_A,			Z80_PARAM_REG8_MEMHL,	-1,		0,		0 },
	{ L"or",	3,	0xB6,	Z80_PARAM_A,			Z80_PARAM_MEMIX_MEMIY,	-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"or",	2,	0xF6,	Z80_PARAM_A,			Z80_PARAM_IMMEDIATE,	-1,		-1,		Z80_IMMEDIATE_U8 },
	{ L"or",	1,	0xB0,	Z80_PARAM_REG8_MEMHL,	Z80_PARAM_NONE,			0,		-1,		0 },
	{ L"or",	3,	0xB6,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"or",	2,	0xF6,	Z80_PARAM_IMMEDIATE,	Z80_PARAM_NONE,			-1,		-1,		Z80_IMMEDIATE_U8 },
	{ L"cp",	1,	0xB8,	Z80_PARAM_A,			Z80_PARAM_REG8_MEMHL,	-1,		0,		0 },
	{ L"cp",	3,	0xBE,	Z80_PARAM_A,			Z80_PARAM_MEMIX_MEMIY,	-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"cp",	2,	0xFE,	Z80_PARAM_A,			Z80_PARAM_IMMEDIATE,	-1,		-1,		Z80_IMMEDIATE_U8 },
	{ L"cp",	1,	0xB8,	Z80_PARAM_REG8_MEMHL,	Z80_PARAM_NONE,			0,		-1,		0 },
	{ L"cp",	3,	0xBE,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"cp",	2,	0xFE,	Z80_PARAM_IMMEDIATE,	Z80_PARAM_NONE,			-1,		-1,		Z80_IMMEDIATE_U8 },
	{ L"inc",	1,	0x04,	Z80_PARAM_REG8_MEMHL,	Z80_PARAM_NONE,			3,		-1,		0 },
	{ L"inc",	1,	0x03,	Z80_PARAM_REG16_SP,		Z80_PARAM_NONE,			4,		-1,		0 },
	{ L"inc",	2,	0x23,	Z80_PARAM_IX_IY,		Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY },
	{ L"inc",	3,	0x34,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"dec",	1,	0x05,	Z80_PARAM_REG8_MEMHL,	Z80_PARAM_NONE,			3,		-1,		0 },
	{ L"dec",	1,	0x0B,	Z80_PARAM_REG16_SP,		Z80_PARAM_NONE,			4,		-1,		0 },
	{ L"dec",	2,	0x2B,	Z80_PARAM_IX_IY,		Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY },
	{ L"dec",	3,	0x35,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY | Z80_IMMEDIATE_U8 },
	{ L"daa",	1,	0x27,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		0 },
	{ L"cpl",	1,	0x2F,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		0 },
	{ L"rlca",	1,	0x07,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		0 },
	{ L"rla",	1,	0x17,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		0 },
	{ L"rrca",	1,	0x0F,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		0 },
	{ L"rra",	1,	0x1F,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		0 },
	{ L"rlc",	2,	0x00,	Z80_PARAM_REG8_MEMHL,	Z80_PARAM_NONE,			0,		-1,		Z80_PREFIX_CB },
	{ L"rlc",	4,	0x06,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_NONE,			-1,		-1,		Z80_PREFIX_CB | Z80_IMMEDIATE_U8 | Z80_Z80 | Z80_PREFIX_IX_IY },
	{ L"rrc",	2,	0x08,	Z80_PARAM_REG8_MEMHL,	Z80_PARAM_NONE,			0,		-1,		Z80_PREFIX_CB },
	{ L"rrc",	4,	0x0E,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_NONE,			-1,		-1,		Z80_PREFIX_CB | Z80_IMMEDIATE_U8 | Z80_Z80 | Z80_PREFIX_IX_IY },
	{ L"rl",	2,	0x10,	Z80_PARAM_REG8_MEMHL,	Z80_PARAM_NONE,			0,		-1,		Z80_PREFIX_CB },
	{ L"rl",	4,	0x16,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_NONE,			-1,		-1,		Z80_PREFIX_CB | Z80_IMMEDIATE_U8 | Z80_Z80 | Z80_PREFIX_IX_IY },
	{ L"rr",	2,	0x18,	Z80_PARAM_REG8_MEMHL,	Z80_PARAM_NONE,			0,		-1,		Z80_PREFIX_CB },
	{ L"rr",	4,	0x1E,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_NONE,			-1,		-1,		Z80_PREFIX_CB | Z80_IMMEDIATE_U8 | Z80_Z80 | Z80_PREFIX_IX_IY },
	{ L"sla",	2,	0x20,	Z80_PARAM_REG8_MEMHL,	Z80_PARAM_NONE,			0,		-1,		Z80_PREFIX_CB },
	{ L"sla",	4,	0x26,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_NONE,			-1,		-1,		Z80_PREFIX_CB | Z80_IMMEDIATE_U8 | Z80_Z80 | Z80_PREFIX_IX_IY },
	{ L"sra",	2,	0x28,	Z80_PARAM_REG8_MEMHL,	Z80_PARAM_NONE,			0,		-1,		Z80_PREFIX_CB },
	{ L"sra",	4,	0x2E,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_NONE,			-1,		-1,		Z80_PREFIX_CB | Z80_IMMEDIATE_U8 | Z80_Z80 | Z80_PREFIX_IX_IY },
	{ L"swap",	2,	0x30,	Z80_PARAM_REG8_MEMHL,	Z80_PARAM_NONE,			0,		-1,		Z80_PREFIX_CB | Z80_GAMEBOY },
	{ L"srl",	2,	0x38,	Z80_PARAM_REG8_MEMHL,	Z80_PARAM_NONE,			0,		-1,		Z80_PREFIX_CB },
	{ L"srl",	4,	0x3E,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_NONE,			-1,		-1,		Z80_PREFIX_CB | Z80_IMMEDIATE_U8 | Z80_Z80 | Z80_PREFIX_IX_IY },
	{ L"bit",	2,	0x40,	Z80_PARAM_IMMEDIATE,	Z80_PARAM_REG8_MEMHL,	3,		0,		Z80_PREFIX_CB | Z80_IMMEDIATE_U3 },
	{ L"bit",	4,	0x46,	Z80_PARAM_IMMEDIATE,	Z80_PARAM_MEMIX_MEMIY,	3,		-1,		Z80_PREFIX_CB | Z80_IMMEDIATE_U3 | Z80_IMMEDIATE2_U8 | Z80_PREFIX_IX_IY },
	{ L"res",	2,	0x80,	Z80_PARAM_IMMEDIATE,	Z80_PARAM_REG8_MEMHL,	3,		0,		Z80_PREFIX_CB | Z80_IMMEDIATE_U3 },
	{ L"res",	4,	0x86,	Z80_PARAM_IMMEDIATE,	Z80_PARAM_MEMIX_MEMIY,	3,		-1,		Z80_PREFIX_CB | Z80_IMMEDIATE_U3 | Z80_IMMEDIATE2_U8 | Z80_PREFIX_IX_IY },
	{ L"set",	2,	0xC0,	Z80_PARAM_IMMEDIATE,	Z80_PARAM_REG8_MEMHL,	3,		0,		Z80_PREFIX_CB | Z80_IMMEDIATE_U3 },
	{ L"set",	4,	0xC6,	Z80_PARAM_IMMEDIATE,	Z80_PARAM_MEMIX_MEMIY,	3,		-1,		Z80_PREFIX_CB | Z80_IMMEDIATE_U3 | Z80_IMMEDIATE2_U8 | Z80_PREFIX_IX_IY },
	{ L"ccf",	1,	0x3F,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		0 },
	{ L"scf",	1,	0x37,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		0 },
	{ L"halt",	1,	0x76,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		0 },
	{ L"stop",	2,	0x10,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_STOP | Z80_GAMEBOY },
	{ L"di",	1,	0xF3,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		0 },
	{ L"ei",	1,	0xFB,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		0 },
	{ L"jp",	3,	0xC2,	Z80_PARAM_CONDITION,	Z80_PARAM_IMMEDIATE,	3,		-1,		Z80_IMMEDIATE_U16 },
	{ L"jp",	1,	0xE9,	Z80_PARAM_MEMHL,		Z80_PARAM_NONE,			-1,		-1,		0 },
	{ L"jp",	1,	0xE9,	Z80_PARAM_HL,			Z80_PARAM_NONE,			-1,		-1,		0 },
	{ L"jp",	2,	0xE9,	Z80_PARAM_MEMIX_MEMIY,	Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY },
	{ L"jp",	2,	0xE9,	Z80_PARAM_IX_IY,		Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY },
	{ L"jp",	3,	0xC3,	Z80_PARAM_IMMEDIATE,	Z80_PARAM_NONE,			-1,		-1,		Z80_IMMEDIATE_U16 },
	{ L"jr",	2,	0x20,	Z80_PARAM_CONDITION,	Z80_PARAM_IMMEDIATE,	3,		-1,		Z80_IMMEDIATE_S8 | Z80_JUMP_RELATIVE },
	{ L"jr",	2,	0x18,	Z80_PARAM_IMMEDIATE,	Z80_PARAM_NONE,			-1,		-1,		Z80_IMMEDIATE_S8 | Z80_JUMP_RELATIVE },
	{ L"call",	3,	0xC4,	Z80_PARAM_CONDITION,	Z80_PARAM_IMMEDIATE,	3,		-1,		Z80_IMMEDIATE_U16 },
	{ L"call",	3,	0xCD,	Z80_PARAM_IMMEDIATE,	Z80_PARAM_NONE,			-1,		-1,		Z80_IMMEDIATE_U16 },
	{ L"ret",	1,	0xC0,	Z80_PARAM_CONDITION,	Z80_PARAM_NONE,			3,		-1,		0 },
	{ L"ret",	1,	0xC9,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		0 },
	{ L"reti",	1,	0xD9,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_GAMEBOY },
	{ L"reti",	2,	0x4D,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"rst",	1,	0xC7,	Z80_PARAM_IMMEDIATE,	Z80_PARAM_NONE,			0,		-1,		Z80_RST },
	{ L"ex",	1,	0x08,	Z80_PARAM_AF,			Z80_PARAM_AF_PRIME,		-1,		-1,		Z80_Z80 },
	{ L"ex",	1,	0xE3,	Z80_PARAM_MEMSP,		Z80_PARAM_HL,			-1,		-1,		Z80_Z80 },
	{ L"ex",	1,	0xEB,	Z80_PARAM_DE,			Z80_PARAM_HL,			-1,		-1,		Z80_Z80 },
	{ L"ex",	2,	0xE3,	Z80_PARAM_MEMSP,		Z80_PARAM_IX_IY,		-1,		-1,		Z80_Z80 | Z80_PREFIX_IX_IY },
	{ L"exx",	1,	0xD9,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 },
	{ L"djnz",	2,	0x10,	Z80_PARAM_IMMEDIATE,	Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_IMMEDIATE_S8 | Z80_JUMP_RELATIVE },
	{ L"out",	2,	0x41,	Z80_PARAM_MEMC,			Z80_PARAM_REG8,			-1,		3,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"out",	2,	0xD3,	Z80_PARAM_MEMIMMEDIATE,	Z80_PARAM_A,			-1,		-1,		Z80_Z80 | Z80_IMMEDIATE_U8 },
	{ L"in",	2,	0x40,	Z80_PARAM_REG8,			Z80_PARAM_MEMC,			3,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"in",	2,	0xDB,	Z80_PARAM_A,			Z80_PARAM_MEMIMMEDIATE,	-1,		-1,		Z80_Z80 | Z80_IMMEDIATE_U8 },
	{ L"neg",	2,	0x44,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"retn",	2,	0x45,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"im",	2,	0x46,	Z80_PARAM_IMMEDIATE,	Z80_PARAM_NONE,			3,		-1,		Z80_Z80 | Z80_PREFIX_ED | Z80_INTERRUPT_MODE },
	{ L"rrd",	2,	0x67,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"rld",	2,	0x6F,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"ldi",	2,	0xA0,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"cpi",	2,	0xA1,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"ini",	2,	0xA2,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"outi",	2,	0xA3,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"ldd",	2,	0xA8,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"cpd",	2,	0xA9,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"ind",	2,	0xAA,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"outd",	2,	0xAB,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"ldir",	2,	0xB0,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"cpir",	2,	0xB1,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"inir",	2,	0xB2,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"otir",	2,	0xB3,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"lddr",	2,	0xB8,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"cpdr",	2,	0xB9,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"indr",	2,	0xBA,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ L"otdr",	2,	0xBB,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		Z80_Z80 | Z80_PREFIX_ED },
	{ nullptr,	0,	0x00,	Z80_PARAM_NONE,			Z80_PARAM_NONE,			-1,		-1,		0 },
};