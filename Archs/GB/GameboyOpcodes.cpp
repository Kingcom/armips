#include "Archs/GB/GameboyOpcodes.h"

// Order:
// - Everything else
// - SP_IMM
// - MEMIMMEDIATE
// - IMMEDIATE
const tGameboyOpcode GameboyOpcodes[] = {
	//	Name	Len	Encode	Left param				Right param				LShift	RShift	Flags
	{ L"nop",	1,	0x00,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
	{ L"ld",	1,	0x40,	GB_PARAM_REG8_MEMHL,	GB_PARAM_REG8_MEMHL,	3,		0,		GB_LOAD_REG8_REG8 },
	{ L"ld",	1,	0x02,	GB_PARAM_MEMBC_MEMDE,	GB_PARAM_A,				4,		-1,		0 },
	{ L"ld",	1,	0x0A,	GB_PARAM_A,				GB_PARAM_MEMBC_MEMDE,	-1,		4,		0 },
	{ L"ld",	1,	0x22,	GB_PARAM_HLI_HLD,		GB_PARAM_A,				4,		-1,		0 },
	{ L"ld",	1,	0x2A,	GB_PARAM_A,				GB_PARAM_HLI_HLD,		-1,		4,		0 },
	{ L"ld",	1,	0xE2,	GB_PARAM_FF00_C,		GB_PARAM_A,				-1,		-1,		0 },
	{ L"ld",	1,	0xF2,	GB_PARAM_A,				GB_PARAM_FF00_C,		-1,		-1,		0 },
	{ L"ld",	1,	0xF9,	GB_PARAM_SP,			GB_PARAM_HL,			-1,		-1,		0 },
	{ L"ld",	2,	0xF8,	GB_PARAM_HL,			GB_PARAM_SP_IMM,		-1,		-1,		GB_IMMEDIATE_S8 },
	{ L"ld",	3,	0xFA,	GB_PARAM_A,				GB_PARAM_MEMIMMEDIATE,	-1,		-1,		GB_IMMEDIATE_U16 },
	{ L"ld",	2,	0x06,	GB_PARAM_REG8_MEMHL,	GB_PARAM_IMMEDIATE,		3,		-1,		GB_IMMEDIATE_U8 },
	{ L"ld",	3,	0x01,	GB_PARAM_REG16_SP,		GB_PARAM_IMMEDIATE,		4,		-1,		GB_IMMEDIATE_U16 },
	{ L"ld",	3,	0x08,	GB_PARAM_MEMIMMEDIATE,	GB_PARAM_SP,			-1,		-1,		GB_IMMEDIATE_U16 },
	{ L"ld",	3,	0xEA,	GB_PARAM_MEMIMMEDIATE,	GB_PARAM_A,				-1,		-1,		GB_IMMEDIATE_U16 },
	{ L"ldi",	1,	0x22,	GB_PARAM_MEMHL,			GB_PARAM_A,				-1,		-1,		0 },
	{ L"ldi",	1,	0x2A,	GB_PARAM_A,				GB_PARAM_MEMHL,			-1,		-1,		0 },
	{ L"ldd",	1,	0x32,	GB_PARAM_MEMHL,			GB_PARAM_A,				-1,		-1,		0 },
	{ L"ldd",	1,	0x3A,	GB_PARAM_A,				GB_PARAM_MEMHL,			-1,		-1,		0 },
	{ L"push",	1,	0xC5,	GB_PARAM_REG16_AF,		GB_PARAM_NONE,			4,		-1,		0 },
	{ L"pop",	1,	0xC1,	GB_PARAM_REG16_AF,		GB_PARAM_NONE,			4,		-1,		0 },
	{ L"add",	1,	0x09,	GB_PARAM_HL,			GB_PARAM_REG16_SP,		-1,		4,		0 },
	{ L"add",	1,	0x80,	GB_PARAM_A,				GB_PARAM_REG8_MEMHL,	-1,		0,		0 },
	{ L"add",	2,	0xC6,	GB_PARAM_A,				GB_PARAM_IMMEDIATE,		-1,		-1,		GB_IMMEDIATE_U8 | GB_ADD_SUB_IMMEDIATE },
	{ L"add",	2,	0xE8,	GB_PARAM_SP,			GB_PARAM_IMMEDIATE,		-1,		-1,		GB_IMMEDIATE_S8 },
	{ L"adc",	1,	0x88,	GB_PARAM_A,				GB_PARAM_REG8_MEMHL,	-1,		0,		0 },
	{ L"adc",	2,	0xCE,	GB_PARAM_A,				GB_PARAM_IMMEDIATE,		-1,		-1,		GB_IMMEDIATE_U8 | GB_ADD_SUB_IMMEDIATE },
	{ L"sub",	1,	0x90,	GB_PARAM_A,				GB_PARAM_REG8_MEMHL,	-1,		0,		0 },
	{ L"sub",	2,	0xD6,	GB_PARAM_A,				GB_PARAM_IMMEDIATE,		-1,		-1,		GB_IMMEDIATE_U8 | GB_ADD_SUB_IMMEDIATE },
	{ L"sub",	2,	0xE8,	GB_PARAM_SP,			GB_PARAM_IMMEDIATE,		-1,		-1,		GB_IMMEDIATE_S8 | GB_NEGATE_IMM },
	{ L"sbc",	1,	0x98,	GB_PARAM_A,				GB_PARAM_REG8_MEMHL,	-1,		0,		0 },
	{ L"sbc",	2,	0xDE,	GB_PARAM_A,				GB_PARAM_IMMEDIATE,		-1,		-1,		GB_IMMEDIATE_U8 | GB_ADD_SUB_IMMEDIATE },
	{ L"and",	1,	0xA0,	GB_PARAM_A,				GB_PARAM_REG8_MEMHL,	-1,		0,		0 },
	{ L"and",	2,	0xE6,	GB_PARAM_A,				GB_PARAM_IMMEDIATE,		-1,		-1,		GB_IMMEDIATE_U8 },
	{ L"xor",	1,	0xA8,	GB_PARAM_A,				GB_PARAM_REG8_MEMHL,	-1,		0,		0 },
	{ L"xor",	2,	0xEE,	GB_PARAM_A,				GB_PARAM_IMMEDIATE,		-1,		-1,		GB_IMMEDIATE_U8 },
	{ L"or",	1,	0xB0,	GB_PARAM_A,				GB_PARAM_REG8_MEMHL,	-1,		0,		0 },
	{ L"or",	2,	0xF6,	GB_PARAM_A,				GB_PARAM_IMMEDIATE,		-1,		-1,		GB_IMMEDIATE_U8 },
	{ L"cp",	1,	0xB8,	GB_PARAM_A,				GB_PARAM_REG8_MEMHL,	-1,		0,		0 },
	{ L"cp",	2,	0xFE,	GB_PARAM_A,				GB_PARAM_IMMEDIATE,		-1,		-1,		GB_IMMEDIATE_U8 },
	{ L"inc",	1,	0x04,	GB_PARAM_REG8_MEMHL,	GB_PARAM_NONE,			3,		-1,		0 },
	{ L"inc",	1,	0x03,	GB_PARAM_REG16_SP,		GB_PARAM_NONE,			4,		-1,		0 },
	{ L"dec",	1,	0x05,	GB_PARAM_REG8_MEMHL,	GB_PARAM_NONE,			3,		-1,		0 },
	{ L"dec",	1,	0x0B,	GB_PARAM_REG16_SP,		GB_PARAM_NONE,			4,		-1,		0 },
	{ L"daa",	1,	0x27,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
	{ L"cpl",	1,	0x2F,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
	{ L"rlca",	1,	0x07,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
	{ L"rla",	1,	0x17,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
	{ L"rrca",	1,	0x0F,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
	{ L"rra",	1,	0x1F,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
	{ L"rlc",	2,	0x00,	GB_PARAM_REG8_MEMHL,	GB_PARAM_NONE,			0,		-1,		GB_PREFIX },
	{ L"rrc",	2,	0x08,	GB_PARAM_REG8_MEMHL,	GB_PARAM_NONE,			0,		-1,		GB_PREFIX },
	{ L"rl",	2,	0x10,	GB_PARAM_REG8_MEMHL,	GB_PARAM_NONE,			0,		-1,		GB_PREFIX },
	{ L"rr",	2,	0x18,	GB_PARAM_REG8_MEMHL,	GB_PARAM_NONE,			0,		-1,		GB_PREFIX },
	{ L"sla",	2,	0x20,	GB_PARAM_REG8_MEMHL,	GB_PARAM_NONE,			0,		-1,		GB_PREFIX },
	{ L"sra",	2,	0x28,	GB_PARAM_REG8_MEMHL,	GB_PARAM_NONE,			0,		-1,		GB_PREFIX },
	{ L"swap",	2,	0x30,	GB_PARAM_REG8_MEMHL,	GB_PARAM_NONE,			0,		-1,		GB_PREFIX },
	{ L"srl",	2,	0x38,	GB_PARAM_REG8_MEMHL,	GB_PARAM_NONE,			0,		-1,		GB_PREFIX },
	{ L"bit",	2,	0x40,	GB_PARAM_IMMEDIATE,		GB_PARAM_REG8_MEMHL,	3,		0,		GB_PREFIX | GB_IMMEDIATE_U3 },
	{ L"res",	2,	0x80,	GB_PARAM_IMMEDIATE,		GB_PARAM_REG8_MEMHL,	3,		0,		GB_PREFIX | GB_IMMEDIATE_U3 },
	{ L"set",	2,	0xC0,	GB_PARAM_IMMEDIATE,		GB_PARAM_REG8_MEMHL,	3,		0,		GB_PREFIX | GB_IMMEDIATE_U3 },
	{ L"ccf",	1,	0x3F,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
	{ L"scf",	1,	0x37,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
	{ L"halt",	1,	0x76,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
	{ L"stop",	2,	0x10,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		GB_STOP },
	{ L"di",	1,	0xF3,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
	{ L"ei",	1,	0xFB,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
//	{ L"jp",	1,	0x00,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
//	{ L"jr",	1,	0x00,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
//	{ L"call",	1,	0x00,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
//	{ L"ret",	1,	0x00,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
//	{ L"reti",	1,	0x00,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
//	{ L"rst",	1,	0x00,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
	{ nullptr,	0,	0x00,	GB_PARAM_NONE,			GB_PARAM_NONE,			-1,		-1,		0 },
};
