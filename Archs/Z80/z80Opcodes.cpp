#include "stdafx.h"
#include "Core/Common.h"
#include "z80Opcodes.h"


/*	Placeholders
	I = 8 bit immediate
	J = 8 bit signed immediate
	B = 4 bit immediate
	K = 16 bit immediate
	N = 8 bit register
	M = 16 bit register
	R = fixed 16 bit register
	Q = fixed 8 bit register
*/

const tZ80Opcode Z80Opcodes[] = {

	{ "nop",	"",				{ 0x00, 0x00 }, 1, 1, 0 },
	{ "ld",		"(M\x03),Q\x00",{ 0x02,	0x00 }, 1, 1, Z80_16BIT_REGISTER_TIMES_16 },
	{ "inc",	"M\x0F",		{ 0x03, 0x00 }, 1, 1, Z80_16BIT_REGISTER_TIMES_16 },	
	{ "inc",	"N\xFF",		{ 0x04, 0x00 }, 1, 1, Z80_8BIT_REGISTER_TIMES_8 },
	{ "dec",	"N\xFF",		{ 0x05, 0x00 }, 1, 1, Z80_8BIT_REGISTER_TIMES_8 },
	{ "ld",		"N\xFF,I",		{ 0x06, 0x00 },	1, 2, Z80_IMMEDIATE|Z80_8BIT_REGISTER_TIMES_8 },
	{ "ld",		"Q\x00,(M\x03)",{ 0x0A,	0x00 }, 1, 1, Z80_16BIT_REGISTER_TIMES_16 },
	{ "ld",		"Q\x00,N\xFF",	{ 0x40, 0x00 },	1, 1, Z80_8BIT_REGISTER_NIBBLE },
	{ "ld",		"Q\x01,N\xFF",	{ 0x48, 0x00 },	1, 1, Z80_8BIT_REGISTER_NIBBLE },
	{ "ld",		"Q\x02,N\xFF",	{ 0x50, 0x00 },	1, 1, Z80_8BIT_REGISTER_NIBBLE },
	{ "ld",		"Q\x03,N\xFF",	{ 0x58, 0x00 },	1, 1, Z80_8BIT_REGISTER_NIBBLE },
	{ "ld",		"Q\x04,N\xFF",	{ 0x60, 0x00 },	1, 1, Z80_8BIT_REGISTER_NIBBLE },
	{ "ld",		"Q\x05,N\xFF",	{ 0x68, 0x00 },	1, 1, Z80_8BIT_REGISTER_NIBBLE },
	{ "ld",		"Q\x06,N\xBF",	{ 0x70, 0x00 },	1, 1, Z80_8BIT_REGISTER_NIBBLE },	// kein (hl),(hl)
	{ "ld",		"Q\x07,N\xFF",	{ 0x78, 0x00 },	1, 1, Z80_8BIT_REGISTER_NIBBLE },
/*

	{ "xor",	"N\xFF",		{ 0xA8, 0x00 }, 1, 1, Z80_8BIT_REGISTER_NIBBLE },
	{ "xor",	"I",		{ 0xEE, 0x00 }, 1, 2, Z80_IMMEDIATE },
	{ "cp",		"N\xFF",		{ 0xB8, 0x00 }, 1, 1, Z80_8BIT_REGISTER_NIBBLE },
	{ "cp",		"I",		{ 0xFE, 0x00 }, 1, 2, Z80_IMMEDIATE },
	{ "add",	"R\x02,M",	{ 0x09, 0x00 }, 1, 1, Z80_16BIT_REGISTER_TIMES_16 },
	{ "add",	"sp,J",		{ 0xE8, 0x00 }, 1, 1, Z80_IMMEDIATE },
	{ "dec",	"M\x0F",		{ 0x0B, 0x00 }, 1, 1, Z80_16BIT_REGISTER_TIMES_16 },

	*/

	{ "daa",	"",			{ 0x27, 0x00 }, 1, 1, 0 },
	{ "cpl",	"",			{ 0x2F, 0x00 }, 1, 1, 0 },
	{ "ccf",	"",			{ 0x3F, 0x00 }, 1, 1, 0 },
	{ "scf",	"",			{ 0x37, 0x00 }, 1, 1, 0 },
	{ "halt",	"",			{ 0x76, 0x00 }, 1, 1, 0 },
	{ "stop",	"",			{ 0x10, 0x00 }, 2, 2, 0 },
	{ "di",		"",			{ 0xF3, 0x00 }, 1, 1, 0 },
	{ "ei",		"",			{ 0xFB, 0x00 }, 1, 1, 0 },
	{ "rlca",	"",			{ 0x07, 0x00 }, 1, 1, 0 },
	{ "rla",	"",			{ 0x17, 0x00 }, 1, 1, 0 },
	{ "rrca",	"",			{ 0x0F, 0x00 }, 1, 1, 0 },
	{ "rra",	"",			{ 0x1F, 0x00 }, 1, 1, 0 },

	{ "swap",	"N\xFF",		{ 0xCB, 0x30 }, 2, 2, Z80_8BIT_REGISTER_NIBBLE },
	{ "rlc",	"N\xFF",		{ 0xCB, 0x00 }, 2, 2, Z80_8BIT_REGISTER_NIBBLE },
	{ "rl",		"N\xFF",		{ 0xCB, 0x10 }, 2, 2, Z80_8BIT_REGISTER_NIBBLE },
	{ "rrc",	"N\xFF",		{ 0xCB, 0x08 }, 2, 2, Z80_8BIT_REGISTER_NIBBLE },
	{ "rr",		"N\xFF",		{ 0xCB, 0x18 }, 2, 2, Z80_8BIT_REGISTER_NIBBLE },
	{ "sla",	"N\xFF",		{ 0xCB, 0x20 }, 2, 2, Z80_8BIT_REGISTER_NIBBLE },
	{ "sra",	"N\xFF",		{ 0xCB, 0x28 }, 2, 2, Z80_8BIT_REGISTER_NIBBLE },
	{ "srl",	"N\xFF",		{ 0xCB, 0x38 }, 2, 2, Z80_8BIT_REGISTER_NIBBLE },
	{ "bit",	"I\x04,N\xFF",	{ 0xCB, 0x40 }, 2, 2, Z80_ADD_IMMEDIATE_TIMES_8|Z80_8BIT_REGISTER_NIBBLE },
	{ "set",	"I\x04,N\xFF",	{ 0xCB, 0xC0 }, 2, 2, Z80_ADD_IMMEDIATE_TIMES_8|Z80_8BIT_REGISTER_NIBBLE },
	{ "res",	"I\x04,N\xFF",	{ 0xCB, 0x80 }, 2, 2, Z80_ADD_IMMEDIATE_TIMES_8|Z80_8BIT_REGISTER_NIBBLE },
/*
	{ "jp",		"K",		{ 0xC2, 0x00 }, 1, 3, 0 },
	{ "jp",		"nz,K",		{ 0xC3, 0x00 }, 1, 3, 0 },
	{ "jp",		"z,K",		{ 0xCA, 0x00 }, 1, 3, 0 },
	{ "jp",		"nc,K",		{ 0xD2, 0x00 }, 1, 3, 0 },
	{ "jp",		"c,K",		{ 0xDA, 0x00 }, 1, 3, 0 },
	{ "jp",		"(hl)",		{ 0xE9, 0x00 }, 1, 1, 0 },
*/
	{ NULL,		NULL,	0 }
};

