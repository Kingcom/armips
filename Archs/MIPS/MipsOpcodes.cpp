#include "stdafx.h"
#include "MipsOpcodes.h"

/* Placeholders for encoding

	s	source register
	d	destination register
	t	target register
	S	float source reg
	D	float dest reg
	T	float traget reg
	i	16 bit immediate value
	I	32 bit immediate value
	u	Shifted 16 bit immediate (upper)
	n	negative 16 bit immediate (for subi/u aliases)
	b	26 bit immediate
	a	5 bit immediate
*/


const tMipsOpcode MipsOpcodes[] = {
//     31---------26------------------------------------------5--------0
//     |=   SPECIAL|                                         | function|
//     ------6----------------------------------------------------6-----
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
// 000 | SLL   | ---   | SRL*1 | SRA   | SLLV  |  ---  | SRLV*2| SRAV  |
// 001 | JR    | JALR  | MOVZ  | MOVN  |SYSCALL| BREAK |  ---  | SYNC  |
// 010 | MFHI  | MTHI  | MFLO  | MTLO  | DSLLV |  ---  |   *3  |  *4   |
// 011 | MULT  | MULTU | DIV   | DIVU  | MADD  | MADDU | ----  | ----- |
// 100 | ADD   | ADDU  | SUB   | SUBU  | AND   | OR    | XOR   | NOR   |
// 101 | mfsa  | mtsa  | SLT   | SLTU  |  *5   |  *6   |  *7   |  *8   |
// 110 | TGE   | TGEU  | TLT   | TLTU  | TEQ   |  ---  | TNE   |  ---  |
// 111 | dsll  |  ---  | dsrl  | dsra  |dsll32 |  ---  |dsrl32 |dsra32 |
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
// *1:	rotr when rs = 1 (PSP only)		*2:	rotrv when rs = 1 (PSP only)
// *3:	dsrlv on PS2, clz on PSP		*4:	dsrav on PS2, clo on PSP
// *5:	dadd on PS2, max on PSP			*6:	daddu on PS2, min on PSP
// *7:	dsub on PS2, msub on PSP		*8:	dsubu on PS2, msubu on PSP
	{ "sll",	"d,t,a",	0x00000000,	MARCH_ALL,	O_RD|O_RT|O_I5 },
	{ "sll",	"d,a",		0x00000000,	MARCH_ALL,	O_RDT|O_I5 },
	{ "nop",	"",			0x00000000,	MARCH_ALL,	0 },
	{ "srl",	"d,t,a",	0x00000002,	MARCH_ALL,	O_RD|O_RT|O_I5 },
	{ "srl",	"d,a",		0x00000002,	MARCH_ALL,	O_RDT|O_I5 },
	{ "rotr",	"d,t,a",	0x00200002,	MARCH_PSP,	O_RD|O_RT|O_I5 },
	{ "rotr",	"d,a",		0x00200002,	MARCH_PSP,	O_RDT|O_I5 },
	{ "sra",	"d,t,a",	0x00000003,	MARCH_ALL,	O_RD|O_RT|O_I5 },
	{ "sra",	"d,a",		0x00000003,	MARCH_ALL,	O_RDT|O_I5 },
	{ "sllv",	"d,t,s",	0x00000004,	MARCH_ALL,	O_RD|O_RT|O_RS },
	{ "sllv",	"d,s",		0x00000004,	MARCH_ALL,	O_RDT|O_RS },
	{ "srlv",	"d,t,s",	0x00000006, MARCH_ALL,	O_RD|O_RT|O_RS },
	{ "srlv",	"d,s",		0x00000006, MARCH_ALL,	O_RDT|O_RS },
	{ "rotrv",	"d,t,s",	0x00200006, MARCH_PSP,	O_RD|O_RT|O_RS },
	{ "rotrv",	"d,s",		0x00200006, MARCH_PSP,	O_RDT|O_RS },
	{ "srav",	"d,t,s",	0x00000007, MARCH_ALL,	O_RD|O_RT|O_RS },
	{ "srav",	"d,s",		0x00000007, MARCH_ALL,	O_RDT|O_RS },
	{ "jr",		"s",		0x00000008,	MARCH_ALL,	O_RS|MO_DELAY|MO_NODELAY },
	{ "jalr",	"s,d",		0x00000009,	MARCH_ALL,	O_RD|O_RS|MO_DELAY|MO_NODELAY },
	{ "jalr",	"s",		0x0000F809,	MARCH_ALL,	O_RS|MO_DELAY|MO_NODELAY },
	{ "movz",	"d,s,t",	0x0000000A, MARCH_LV2,	O_RD|O_RT|O_RS },
	{ "movn",	"d,s,t",	0x0000000B, MARCH_LV2,	O_RD|O_RT|O_RS },
	{ "syscall","b",		0x0000000c,	MARCH_ALL,	O_I20|MO_NODELAY },
	{ "break",	"b",		0x0000000d,	MARCH_ALL,	O_I20|MO_NODELAY },
	{ "sync",	"",			0x0000000f,	MARCH_LV2,	0 },
	{ "mfhi",	"d",		0x00000010,	MARCH_ALL,	O_RD },
	{ "mthi",	"s",		0x00000011,	MARCH_ALL,	O_RS },
	{ "mflo",	"d",		0x00000012,	MARCH_ALL,	O_RD },
	{ "mtlo",	"s",		0x00000013,	MARCH_ALL,	O_RS },
	{ "dsllv",	"d,t,s",	0x00000014,	MARCH_PS2,	O_RD|O_RT|O_RS },
	{ "dsllv",	"d,s",		0x00000014,	MARCH_PS2,	O_RT|O_RS },
	{ "dsrlv",	"d,t,s",	0x00000016,	MARCH_PS2,	O_RD|O_RT|O_RS },
	{ "dsrlv",	"d,s",		0x00000016,	MARCH_PS2,	O_RDT|O_RS },
	{ "clz",	"d,s",		0x00000016,	MARCH_PSP,	O_RD|O_RS },
	{ "dsrav",	"d,t,s",	0x00000017,	MARCH_PS2,	O_RD|O_RT|O_RS },
	{ "dsrav",	"d,s",		0x00000017,	MARCH_PS2,	O_RDT|O_RS },
	{ "clo",	"d,s",		0x00000017,	MARCH_PSP,	O_RD|O_RS },
	{ "mult",	"s,t",		0x00000018,	MARCH_ALL,	O_RS|O_RT },
	{ "mult",	"r\x0,s,t",	0x00000018,	MARCH_ALL,	O_RS|O_RT },
	{ "multu",	"s,t",		0x00000019,	MARCH_ALL,	O_RS|O_RT },
	{ "multu",	"r\x0,s,t",	0x00000019,	MARCH_ALL,	O_RS|O_RT },
	{ "div",	"s,t",		0x0000001a,	MARCH_ALL,	O_RS|O_RT },
	{ "div",	"r\x0,s,t",	0x0000001a,	MARCH_ALL,	O_RS|O_RT },
	{ "divu",	"s,t",		0x0000001b,	MARCH_ALL,	O_RS|O_RT },
	{ "divu",	"r\x0,s,t",	0x0000001b,	MARCH_ALL,	O_RS|O_RT },
	{ "madd",	"s,t",		0x0000001c,	MARCH_PSP,	O_RS|O_RT },
	{ "maddu",	"s,t",		0x0000001d,	MARCH_PSP,	O_RS|O_RT },
	{ "add",	"d,s,t",	0x00000020,	MARCH_ALL,	O_RD|O_RS|O_RT },
	{ "add",	"s,t",		0x00000020,	MARCH_ALL,	O_RSD|O_RT },
	{ "addu",	"d,s,t",	0x00000021,	MARCH_ALL,	O_RD|O_RS|O_RT },
	{ "addu",	"s,t",		0x00000021,	MARCH_ALL,	O_RSD|O_RT },
	{ "move",	"d,s",		0x00000021,	MARCH_ALL,	O_RD|O_RS },
	{ "sub",	"d,s,t",	0x00000022,	MARCH_ALL,	O_RD|O_RS|O_RT },
	{ "sub",	"s,t",		0x00000022,	MARCH_ALL,	O_RSD|O_RT },
	{ "neg",	"d,t",		0x00000022,	MARCH_ALL,	O_RD|O_RT },
	{ "subu",	"d,s,t",	0x00000023,	MARCH_ALL,	O_RD|O_RS|O_RT },
	{ "subu",	"s,t",		0x00000023,	MARCH_ALL,	O_RSD|O_RT },
	{ "negu",	"d,t",		0x00000023,	MARCH_ALL,	O_RD|O_RT },
	{ "and",	"d,s,t",	0x00000024,	MARCH_ALL,	O_RD|O_RS|O_RT },
	{ "and",	"s,t",		0x00000024,	MARCH_ALL,	O_RSD|O_RT },
	{ "or",		"d,s,t",	0x00000025,	MARCH_ALL,	O_RS|O_RT|O_RD },
	{ "or",		"s,t",		0x00000025,	MARCH_ALL,	O_RSD|O_RT },
	{ "xor",	"d,s,t",	0x00000026, MARCH_ALL,	O_RS|O_RD|O_RT },
	{ "eor",	"d,s,t",	0x00000026, MARCH_ALL,	O_RS|O_RD|O_RT },
	{ "xor",	"s,t",		0x00000026, MARCH_ALL,	O_RSD|O_RT },
	{ "eor",	"s,t",		0x00000026, MARCH_ALL,	O_RSD|O_RT },
	{ "nor",	"d,s,t",	0x00000027,	MARCH_ALL,	O_RS|O_RT|O_RD },
	{ "nor",	"s,t",		0x00000027,	MARCH_ALL,	O_RSD|O_RT },
	{ "mfsa",	"d",		0x00000028,	MARCH_PS2,	O_RD },
	{ "mtsa",	"s",		0x00000029,	MARCH_PS2,	O_RS },
	{ "slt",	"d,s,t",	0x0000002a,	MARCH_ALL,	O_RD|O_RT|O_RS },
	{ "slt",	"s,t",		0x0000002a,	MARCH_ALL,	O_RSD|O_RT},
	{ "sltu",	"d,s,t",	0x0000002b,	MARCH_ALL,	O_RD|O_RT|O_RS },
	{ "sltu",	"s,t",		0x0000002b,	MARCH_ALL,	O_RSD|O_RT },
	{ "dadd",	"d,s,t",	0x0000002C, MARCH_PS2,	O_RD|O_RT|O_RS },
	{ "max",	"d,s,t",	0x0000002C, MARCH_PSP,	O_RD|O_RT|O_RS },
	{ "daddu",	"d,s,t",	0x0000002D, MARCH_PS2,	O_RD|O_RT|O_RS },
	{ "min",	"d,s,t",	0x0000002D, MARCH_PSP,	O_RD|O_RT|O_RS },
	{ "dsub",	"d,s,t",	0x0000002E, MARCH_PS2,	O_RD|O_RT|O_RS },
	{ "msub",	"s,t",		0x0000002E,	MARCH_PSP,	O_RS|O_RT },
	{ "dsubu",	"d,s,t",	0x0000002F, MARCH_PS2,	O_RD|O_RT|O_RS },
	{ "msubu",	"s,t",		0x0000002F,	MARCH_PSP,	O_RS|O_RT },
	{ "tge",	"s,t",		0x00000030,	MARCH_PS2,	O_RSD|O_RT },
	{ "tgeu",	"s,t",		0x00000031,	MARCH_PS2,	O_RSD|O_RT },
	{ "tlt",	"s,t",		0x00000032,	MARCH_PS2,	O_RSD|O_RT },
	{ "tltu",	"s,t",		0x00000033,	MARCH_PS2,	O_RSD|O_RT },
	{ "teq",	"s,t",		0x00000034,	MARCH_PS2,	O_RSD|O_RT },
	{ "tne",	"s,t",		0x00000036,	MARCH_PS2,	O_RSD|O_RT },
	{ "dsll",	"d,t,a",	0x00000038,	MARCH_PS2,	O_RD|O_RT|O_I5 },
	{ "dsll",	"d,a",		0x00000038,	MARCH_PS2,	O_RDT|O_I5 },
	{ "dsrl",	"d,t,a",	0x0000003A,	MARCH_PS2,	O_RD|O_RT|O_I5 },
	{ "dsrl",	"d,a",		0x0000003A,	MARCH_PS2,	O_RDT|O_I5 },
	{ "dsra",	"d,t,a",	0x0000003B,	MARCH_PS2,	O_RD|O_RT|O_I5 },
	{ "dsra",	"d,a",		0x0000003B,	MARCH_PS2,	O_RDT|O_I5 },
	{ "dsll32",	"d,t,a",	0x0000003C,	MARCH_PS2,	O_RD|O_RT|O_I5 },
	{ "dsll32",	"d,a",		0x0000003C,	MARCH_PS2,	O_RDT|O_I5 },
	{ "dsrl32",	"d,t,a",	0x0000003E,	MARCH_PS2,	O_RD|O_RT|O_I5 },
	{ "dsrl32",	"d,a",		0x0000003E,	MARCH_PS2,	O_RDT|O_I5 },
	{ "dsra32",	"d,t,a",	0x0000003F,	MARCH_PS2,	O_RD|O_RT|O_I5 },
	{ "dsra32",	"d,a",		0x0000003F,	MARCH_PS2,	O_RDT|O_I5 },

//     REGIMM: encoded by the rt field when opcode field = REGIMM.
//     31---------26----------20-------16------------------------------0
//     |=    REGIMM|          |   rt    |                              |
//     ------6---------------------5------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 | BLTZ  | BGEZ  | BLTZL | BGEZL |  ---  |  ---  |  ---  |  ---  |
//  01 | tgei  | tgeiu | tlti  | tltiu | teqi  |  ---  | tnei  |  ---  |
//  10 | BLTZAL| BGEZAL|BLTZALL|BGEZALL|  ---  |  ---  |  ---  |  ---  |
//  11 | mtsab | mtsah |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "bltz",	"s,I",		0x04000000,	MARCH_ALL,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bgez",	"s,I",		0x04010000, MARCH_ALL,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bltzl",	"s,I",		0x04020000,	MARCH_LV2,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bgezl",	"s,I",		0x04030000, MARCH_LV2,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "tgei",	"s,I",		0x04080000,	MARCH_PS2,	O_RS|O_I16 },
	{ "tgeiu",	"s,I",		0x04090000,	MARCH_PS2,	O_RS|O_I16 },
	{ "tlti",	"s,I",		0x040A0000,	MARCH_PS2,	O_RS|O_I16 },
	{ "tltiu",	"s,I",		0x040B0000,	MARCH_PS2,	O_RS|O_I16 },
	{ "teqi",	"s,I",		0x040C0000,	MARCH_PS2,	O_RS|O_I16 },
	{ "tnei",	"s,I",		0x040E0000,	MARCH_PS2,	O_RS|O_I16 },
	{ "bltzal",	"s,I",		0x04100000, MARCH_ALL,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bgezal",	"s,I",		0x04110000, MARCH_ALL,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bltzall","s,I",		0x04120000, MARCH_LV2,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bgezall","s,I",		0x04130000, MARCH_LV2,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "mtsab",	"s,I",		0x04180000,	MARCH_PS2,	O_RS|O_I16 },
	{ "mtsah",	"s,I",		0x04190000,	MARCH_PS2,	O_RS|O_I16 },

	// OPCODE 02 - J
	{ "j",		"I",		0x08000000, MARCH_ALL,	O_I26|O_IPCA|MO_DELAY|MO_NODELAY },
	{ "b",		"I",		0x08000000, MARCH_ALL,	O_I26|O_IPCA|MO_DELAY|MO_NODELAY },
	// OPCODE 03 - JAL
	{ "jal",	"I",		0x0c000000,	MARCH_ALL,	O_I26|O_IPCA|MO_DELAY|MO_NODELAY },
	// OPCODE 04 - BEQ
	{ "beq",	"s,t,I",	0x10000000,	MARCH_ALL,	O_RS|O_RT|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "beqz",	"s,I",		0x10000000,	MARCH_ALL,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	// OPCODE 05 - BNE
	{ "bne",	"s,t,I",	0x14000000,	MARCH_ALL,	O_RS|O_RT|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bnez",	"s,I",		0x14000000,	MARCH_ALL,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	// OPCODE 06 - BLEZ
	{ "blez",	"s,I",		0x18000000,	MARCH_ALL,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	// OPCODE 07 - BGTZ
	{ "bgtz",	"s,I",		0x1c000000,	MARCH_ALL,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	// OPCODE 08 - ADDI
	{ "addi",	"t,s,i",	0x20000000,	MARCH_ALL,	O_RT|O_RS|O_I16 },
	{ "addi",	"s,i",		0x20000000,	MARCH_ALL,	O_RST|O_I16 },
	// OPCODE 09 - ADDIU
	{ "addiu",	"t,s,i",	0x24000000,	MARCH_ALL,	O_RT|O_RS|O_I16 },
	{ "addiu",	"s,i",		0x24000000,	MARCH_ALL,	O_RST|O_I16 },
	// OPCODE 0A - SLTI
	{ "slti",	"t,s,i",	0x28000000,	MARCH_ALL,	O_RT|O_RS|O_I16 },
	{ "slti",	"s,i",		0x28000000,	MARCH_ALL,	O_RST|O_I16 },
	// OPCODE 0B - SLTIU
	{ "sltiu",	"t,s,i",	0x2c000000,	MARCH_ALL,	O_RT|O_RS|O_I16 },
	{ "sltiu",	"s,i",		0x2c000000,	MARCH_ALL,	O_RST|O_I16 },
	// OPCODE 0C - ANDI
	{ "andi",	"t,s,i",	0x30000000,	MARCH_ALL,	O_RT|O_RS|O_I16 },
	{ "andi",	"s,i",		0x30000000,	MARCH_ALL,	O_RST|O_I16 },
	// OPCODE 0D - ORI
	{ "ori",	"t,s,i",	0x34000000,	MARCH_ALL,	O_RS|O_RT|O_I16 },
	{ "ori",	"s,i",		0x34000000,	MARCH_ALL,	O_RST|O_I16 },
	// OPCODE 0E - XORI
	{ "xori",	"t,s,i",	0x38000000,	MARCH_ALL,	O_RT|O_RS|O_I16 },
	{ "xori",	"s,i",		0x38000000,	MARCH_ALL,	O_RST|O_I16 },
	// OPCODE 0F - LUI
	{ "lui",	"t,i",		0x3c000000,	MARCH_ALL,	O_RT|O_I16 },
	// OPCODE 10 - COP0
	// OPCODE 11 - COP1
	// OPCODE 12 - COP2
	// OPCODE 13 - COP3
	// OPCODE 14 - BEQL		MIPS 2
	{ "beql",	"s,t,I",	0x50000000,	MARCH_LV2,	O_RS|O_RT|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "beqzl",	"s,I",		0x50000000,	MARCH_LV2,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	// OPCODE 15 - BNEZL	MIPS 2
	{ "bnel",	"s,t,I",	0x54000000,	MARCH_LV2,	O_RS|O_RT|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bnezl",	"s,I",		0x54000000,	MARCH_LV2,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	// OPCODE 16 - BLEZL	MIPS 2
	{ "blezl",	"s,I",		0x58000000,	MARCH_LV2,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	// OPCODE 17 - BGTZL	MIPS 2
	{ "bgtzl",	"s,I",		0x5c000000,	MARCH_LV2,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	// OPCODE 18 - UNDEF
	// OPCODE 19 - UNDEF
	// OPCODE 1A - UNDEF
	// OPCODE 1B - UNDEF
	// OPCODE 1C - UNDEF
	// OPCODE 1D - UNDEF
	// OPCODE 1E - UNDEF
	// OPCODE 1F - UNDEF
	// OPCODE 20 - LB
	{ "lb",		"t,i(s)",	0x80000000,	MARCH_ALL,	O_RT|O_I16|O_RS|MO_DELAYRT },
	{ "lb",		"t,(s)",	0x80000000,	MARCH_ALL,	O_RT|O_RS|MO_DELAYRT },
	// OPCODE 21 - LH
	{ "lh",		"t,i(s)",	0x84000000,	MARCH_ALL,	O_RT|O_I16|O_RS|MO_DELAYRT },
	{ "lh",		"t,(s)",	0x84000000,	MARCH_ALL,	O_RT|O_RS|MO_DELAYRT },
	// OPCODE 22 - LWL
	{ "lwl",	"t,i(s)",	0x88000000,	MARCH_ALL,	O_RT|O_I16|O_RS|MO_DELAYRT },
	{ "lwl",	"t,(s)",	0x88000000,	MARCH_ALL,	O_RT|O_RS|MO_DELAYRT },
	// OPCODE 23 - LW
	{ "lw",		"t,i(s)",	0x8c000000,	MARCH_ALL,	O_RT|O_I16|O_RS|MO_DELAYRT },
	{ "lw",		"t,(s)",	0x8c000000,	MARCH_ALL,	O_RT|O_RS|MO_DELAYRT },
	// OPCODE 24 - LBU
	{ "lbu",	"t,i(s)",	0x90000000,	MARCH_ALL,	O_RT|O_I16|O_RS|MO_DELAYRT },
	{ "lbu",	"t,(s)",	0x90000000,	MARCH_ALL,	O_RT|O_RS|MO_DELAYRT },
	// OPCODE 25 - LHU
	{ "lhu",	"t,i(s)",	0x94000000,	MARCH_ALL,	O_RT|O_I16|O_RS|MO_DELAYRT },
	{ "lhu",	"t,(s)",	0x94000000,	MARCH_ALL,	O_RT|O_RS|MO_DELAYRT },
	// OPCODE 26 - LWR
	{ "lwr",	"t,i(s)",	0x98000000,	MARCH_ALL,	O_RT|O_I16|O_RS|MO_DELAYRT|MO_IGNORERTD },
	{ "lwr",	"t,(s)",	0x98000000,	MARCH_ALL,	O_RT|O_RS|MO_DELAYRT|MO_IGNORERTD },
	// OPCODE 27 - UNDEF
	// OPCODE 28 - SB
	{ "sb",		"t,i(s)",	0xa0000000,	MARCH_ALL,	O_RT|O_I16|O_RS },
	{ "sb",		"t,(s)",	0xa0000000,	MARCH_ALL,	O_RT|O_RS },
	// OPCODE 29 - SH
	{ "sh",		"t,i(s)",	0xa4000000,	MARCH_ALL,	O_RT|O_I16|O_RS },
	{ "sh",		"t,(s)",	0xa4000000,	MARCH_ALL,	O_RT|O_RS },
	// OPCODE 2A - SWL
	{ "swl",	"t,i(s)",	0xa8000000,	MARCH_ALL,	O_RT|O_I16|O_RS },
	{ "swl",	"t,(s)",	0xa8000000,	MARCH_ALL,	O_RT|O_RS },
	// OPCODE 2B - SW
	{ "sw",		"t,i(s)",	0xac000000,	MARCH_ALL,	O_RT|O_I16|O_RS },
	{ "sw",		"t,(s)",	0xac000000,	MARCH_ALL,	O_RT|O_RS },
	// OPCODE 2C - UNDEF
	// OPCODE 2D - UNDEF
	// OPCODE 2E - SWR
	{ "swr",	"t,i(s)",	0xb8000000,	MARCH_ALL,	O_RT|O_I16|O_RS },
	{ "swr",	"t,(s)",	0xb8000000,	MARCH_ALL,	O_RT|O_RS },
	// OPCODE 2F - UNDEF
	// OPCODE 30 - UNDEF
	// OPCODE 31 - LWC1
	// OPCODE 32 - LWC2
	// OPCODE 33 - LWC3
	// OPCODE 34 - UNDEF
	// OPCODE 35 - UNDEF
	// OPCODE 36 - UNDEF
	// OPCODE 37 - UNDEF
	// OPCODE 38 - UNDEF
	// OPCODE 39 - SWC1
	// OPCODE 3A - SWC2
	// OPCODE 3B - SWC3
	// OPCODE 3C - UNDEF
	// OPCODE 3D - UNDEF
	// OPCODE 3E - UNDEF
	// OPCODE 3F - UNDEF


//     31-------26------21---------------------------------------------0
//     |=    COP1|  rs  |                                              |
//     -----6-------5---------------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 |  MFC1 |  ---  |  CFC1 |  ---  |  MTC1 |  ---  |  CTC1 |  ---  |
//  01 |  BC*  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  10 |  S*   |  ---  |  ---  |  ---  |  W*   |  ---  |  ---  |  ---  |
//  11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "mfc1",	"t,S",		0x44000000,	MARCH_LV2,	O_RT|MO_FRS },
	{ "cfc1",	"t,S",		0x44400000,	MARCH_LV2,	O_RT|MO_FRS },
	{ "mtc1",	"t,S",		0x44800000,	MARCH_LV2,	O_RT|MO_FRS },
	{ "ctc1",	"t,S",		0x44C00000,	MARCH_LV2,	O_RT|MO_FRS },

//     31---------21-------16------------------------------------------0
//     |=    COP1BC|  rt   |                                           |
//     ------11---------5-----------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 |  BC1F | BC1T  | BC1FL | BC1TL |  ---  |  ---  |  ---  |  ---  |
//  01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "bc1f",	"I",		0x45000000,	MARCH_LV2,	O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bc1t",	"I",		0x45010000,	MARCH_LV2,	O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bc1fl",	"I",		0x45020000,	MARCH_LV2,	O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bc1tl",	"I",		0x45030000,	MARCH_LV2,	O_I16|O_IPCR|MO_DELAY|MO_NODELAY },

//     31---------21------------------------------------------5--------0
//     |=  COP1S  |                                          | function|
//     -----11----------------------------------------------------6-----
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
// 000 |  add  |  sub  |  mul  |  div  | sqrt  |  abs  |  mov  |  neg  |
// 001 |  ---  |  ---  |  ---  |  ---  |round.w|trunc.w|ceil.w |floor.w|
// 010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | rsqrt |  ---  |
// 011 |  adda |  suba | mula  |  ---  | madd  |  msub | madda | msuba |
// 100 |  ---  |  ---  |  ---  |  ---  | cvt.w |  ---  |  ---  |  ---  |
// 101 |  max  |  min  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 110 |  c.f  | c.un  | c.eq  | c.ueq |c.(o)lt| c.ult |c.(o)le| c.ule |
// 110 |  c.sf | c.ngle| c.seq | c.ngl | c.lt  | c.nge | c.le  | c.ngt |
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "add.s",		"D,S,T",	0x46000000,	MARCH_LV2,	MO_FRT|MO_FRD|MO_FRS },
	{ "add.s",		"S,T",		0x46000000,	MARCH_LV2,	MO_FRT|MO_FRSD },
	{ "sub.s",		"D,S,T",	0x46000001,	MARCH_LV2,	MO_FRT|MO_FRD|MO_FRS },
	{ "sub.s",		"S,T",		0x46000001,	MARCH_LV2,	MO_FRT|MO_FRSD },
	{ "mul.s",		"D,S,T",	0x46000002,	MARCH_LV2,	MO_FRT|MO_FRD|MO_FRS },
	{ "mul.s",		"S,T",		0x46000002,	MARCH_LV2,	MO_FRT|MO_FRSD },
	{ "div.s",		"D,S,T",	0x46000003,	MARCH_LV2,	MO_FRT|MO_FRD|MO_FRS },
	{ "div.s",		"S,T",		0x46000003,	MARCH_LV2,	MO_FRT|MO_FRSD },
	{ "sqrt.s",		"D,S",		0x46000004,	MARCH_LV2,	MO_FRD|MO_FRS },
	{ "abs.s",		"D,S",		0x46000005,	MARCH_LV2,	MO_FRD|MO_FRS },
	{ "mov.s",		"D,S",		0x46000006,	MARCH_LV2,	MO_FRD|MO_FRS },
	{ "neg.s",		"D,S",		0x46000007,	MARCH_LV2,	MO_FRD|MO_FRS },
	{ "round.w.s",	"D,S",		0x4600000C,	MARCH_PSP,	MO_FRD|MO_FRS },
	{ "trunc.w.s",	"D,S",		0x4600000D,	MARCH_PSP,	MO_FRD|MO_FRS },
	{ "ceil.w.s",	"D,S",		0x4600000E,	MARCH_PSP,	MO_FRD|MO_FRS },
	{ "floor.w.s",	"D,S",		0x4600000F,	MARCH_PSP,	MO_FRD|MO_FRS },
	{ "rsqrt.w.s",	"D,S",		0x46000016,	MARCH_PS2,	MO_FRD|MO_FRS },
	{ "adda.s",		"S,T",		0x46000018, MARCH_PS2,	MO_FRT|MO_FRS },
	{ "suba.s",		"S,T",		0x46000019, MARCH_PS2,	MO_FRT|MO_FRS },
	{ "mula.s",		"S,T",		0x4600001A, MARCH_PS2,	MO_FRT|MO_FRS },
	{ "madd.s",		"D,S,T",	0x4600001C,	MARCH_PS2,	MO_FRT|MO_FRD|MO_FRS },
	{ "madd.s",		"S,T",		0x4600001C,	MARCH_PS2,	MO_FRT|MO_FRSD },
	{ "msub.s",		"D,S,T",	0x4600001D,	MARCH_PS2,	MO_FRT|MO_FRD|MO_FRS },
	{ "msub.s",		"S,T",		0x4600001D,	MARCH_PS2,	MO_FRT|MO_FRSD },
	{ "madda.s",	"S,T",		0x4600001E, MARCH_PS2,	MO_FRT|MO_FRS },
	{ "msuba.s",	"S,T",		0x4600001F, MARCH_PS2,	MO_FRT|MO_FRS },
	{ "cvt.w.s",	"D,S",		0x46000024,	MARCH_LV2,	MO_FRD|MO_FRS },
	{ "max.s",		"D,S,T",	0x46000028,	MARCH_PS2,	MO_FRT|MO_FRD|MO_FRS },
	{ "max.s",		"S,T",		0x46000028,	MARCH_PS2,	MO_FRT|MO_FRSD },
	{ "min.s",		"D,S,T",	0x46000029,	MARCH_PS2,	MO_FRT|MO_FRD|MO_FRS },
	{ "min.s",		"S,T",		0x46000029,	MARCH_PS2,	MO_FRT|MO_FRSD },
	{ "c.f.s",		"S,T",		0x46000030,	MARCH_LV2,	MO_FRT|MO_FRS },
	{ "c.un.s",		"S,T",		0x46000031,	MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.eq.s",		"S,T",		0x46000032,	MARCH_LV2,	MO_FRT|MO_FRS },
	{ "c.ueq.s",	"S,T",		0x46000033,	MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.olt.s",	"S,T",		0x46000034,	MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.lt.s",		"S,T",		0x46000034,	MARCH_PS2,	MO_FRT|MO_FRS },
	{ "c.ult.s",	"S,T",		0x46000035,	MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.ole.s",	"S,T",		0x46000036,	MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.le.s",		"S,T",		0x46000036,	MARCH_PS2,	MO_FRT|MO_FRS },
	{ "c.ule.s",	"S,T",		0x46000037,	MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.sf.s",		"S,T",		0x46000038,	MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.ngle.s",	"S,T",		0x46000039,	MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.seq.s",	"S,T",		0x4600003A,	MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.ngl.s",	"S,T",		0x4600003B,	MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.lt.s",		"S,T",		0x4600003C,	MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.nge.s",	"S,T",		0x4600003D,	MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.le.s",		"S,T",		0x4600003E,	MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.ngt.s",	"S,T",		0x4600003F,	MARCH_PSP,	MO_FRT|MO_FRS },

//     COP1W: encoded by function field
//     31---------21------------------------------------------5--------0
//     |=  COP1W  |                                          | function|
//     -----11----------------------------------------------------6-----
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
// 000 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 001 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 011 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 100 |cvt.s.w|  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 101 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "cvt.s.w",	"D,S",		0x46800020,	MARCH_LV2,	MO_FRD|MO_FRS },




	// END
	{ NULL,		NULL,		0,			0 }
};