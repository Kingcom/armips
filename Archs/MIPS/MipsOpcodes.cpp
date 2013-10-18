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
//     31---------26---------------------------------------------------0
//     |  opcode   |                                                   |
//     ------6----------------------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
// 000 | *1    | *2    | J     | JAL   | BEQ   | BNE   | BLEZ  | BGTZ  | 00..07
// 001 | ADDI  | ADDIU | SLTI  | SLTIU | ANDI  | ORI   | XORI  | LUI   | 08..0F
// 010 | *3    | *4    | ---   | ---   | BEQL  | BNEL  | BLEZL | BGTZL | 10..17
// 011 | ---   | ---   |  ---  | ---   | ---   | ---   | ---   | ---   | 18..1F
// 100 | LB    | LH    | LWL   | LW    | LBU   | LHU   | LWR   | ---   | 20..27
// 101 | SB    | SH    | SWL   | SW    | ---   | ---   | SWR   | CACHE | 28..2F
// 110 | LL    | LWC1  | ---   | ---   | ---   | ---   | ---   | ---   | 30..37
// 111 | SC    | SWC1  | ---   | ---   | ---   | ---   | ---   | ---   | 38..3F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
//		*1 = SPECIAL	*2 = REGIMM		*3 = COP0		*4 = COP1
	{ "j",		"I",		MIPS_OP(0x02), 					MARCH_ALL,	O_I26|O_IPCA|MO_DELAY|MO_NODELAY },
	{ "b",		"I",		MIPS_OP(0x02), 					MARCH_ALL,	O_I26|O_IPCA|MO_DELAY|MO_NODELAY },
	{ "jal",	"I",		MIPS_OP(0x03),					MARCH_ALL,	O_I26|O_IPCA|MO_DELAY|MO_NODELAY },
	{ "beq",	"s,t,I",	MIPS_OP(0x04),					MARCH_ALL,	O_RS|O_RT|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "beqz",	"s,I",		MIPS_OP(0x04),					MARCH_ALL,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bne",	"s,t,I",	MIPS_OP(0x05),					MARCH_ALL,	O_RS|O_RT|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bnez",	"s,I",		MIPS_OP(0x05),					MARCH_ALL,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "blez",	"s,I",		MIPS_OP(0x06),					MARCH_ALL,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bgtz",	"s,I",		MIPS_OP(0x07),					MARCH_ALL,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "addi",	"t,s,i",	MIPS_OP(0x08),					MARCH_ALL,	O_RT|O_RS|O_I16 },
	{ "addi",	"s,i",		MIPS_OP(0x08),					MARCH_ALL,	O_RST|O_I16 },
	{ "addiu",	"t,s,i",	MIPS_OP(0x09),					MARCH_ALL,	O_RT|O_RS|O_I16 },
	{ "addiu",	"s,i",		MIPS_OP(0x09),					MARCH_ALL,	O_RST|O_I16 },
	{ "slti",	"t,s,i",	MIPS_OP(0x0A),					MARCH_ALL,	O_RT|O_RS|O_I16 },
	{ "slti",	"s,i",		MIPS_OP(0x0A),					MARCH_ALL,	O_RST|O_I16 },
	{ "sltiu",	"t,s,i",	MIPS_OP(0x0B),					MARCH_ALL,	O_RT|O_RS|O_I16 },
	{ "sltiu",	"s,i",		MIPS_OP(0x0B),					MARCH_ALL,	O_RST|O_I16 },
	{ "andi",	"t,s,i",	MIPS_OP(0x0C),					MARCH_ALL,	O_RT|O_RS|O_I16 },
	{ "andi",	"s,i",		MIPS_OP(0x0C),					MARCH_ALL,	O_RST|O_I16 },
	{ "ori",	"t,s,i",	MIPS_OP(0x0D),					MARCH_ALL,	O_RS|O_RT|O_I16 },
	{ "ori",	"s,i",		MIPS_OP(0x0D),					MARCH_ALL,	O_RST|O_I16 },
	{ "xori",	"t,s,i",	MIPS_OP(0x0E),					MARCH_ALL,	O_RT|O_RS|O_I16 },
	{ "xori",	"s,i",		MIPS_OP(0x0E),					MARCH_ALL,	O_RST|O_I16 },
	{ "lui",	"t,i",		MIPS_OP(0x0F),					MARCH_ALL,	O_RT|O_I16 },
	{ "beql",	"s,t,I",	MIPS_OP(0x14),					MARCH_LV2,	O_RS|O_RT|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "beqzl",	"s,I",		MIPS_OP(0x14),					MARCH_LV2,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bnel",	"s,t,I",	MIPS_OP(0x15),					MARCH_LV2,	O_RS|O_RT|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bnezl",	"s,I",		MIPS_OP(0x15),					MARCH_LV2,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "blezl",	"s,I",		MIPS_OP(0x16),					MARCH_LV2,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bgtzl",	"s,I",		MIPS_OP(0x17),					MARCH_LV2,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "lb",		"t,i(s)",	MIPS_OP(0x20),					MARCH_ALL,	O_RT|O_I16|O_RS|MO_DELAYRT },
	{ "lb",		"t,(s)",	MIPS_OP(0x20),					MARCH_ALL,	O_RT|O_RS|MO_DELAYRT },
	{ "lh",		"t,i(s)",	MIPS_OP(0x21),					MARCH_ALL,	O_RT|O_I16|O_RS|MO_DELAYRT },
	{ "lh",		"t,(s)",	MIPS_OP(0x21),					MARCH_ALL,	O_RT|O_RS|MO_DELAYRT },
	{ "lwl",	"t,i(s)",	MIPS_OP(0x22),					MARCH_ALL,	O_RT|O_I16|O_RS|MO_DELAYRT },
	{ "lwl",	"t,(s)",	MIPS_OP(0x22),					MARCH_ALL,	O_RT|O_RS|MO_DELAYRT },
	{ "lw",		"t,i(s)",	MIPS_OP(0x23),					MARCH_ALL,	O_RT|O_I16|O_RS|MO_DELAYRT },
	{ "lw",		"t,(s)",	MIPS_OP(0x23),					MARCH_ALL,	O_RT|O_RS|MO_DELAYRT },
	{ "lbu",	"t,i(s)",	MIPS_OP(0x24),					MARCH_ALL,	O_RT|O_I16|O_RS|MO_DELAYRT },
	{ "lbu",	"t,(s)",	MIPS_OP(0x24),					MARCH_ALL,	O_RT|O_RS|MO_DELAYRT },
	{ "lhu",	"t,i(s)",	MIPS_OP(0x25),					MARCH_ALL,	O_RT|O_I16|O_RS|MO_DELAYRT },
	{ "lhu",	"t,(s)",	MIPS_OP(0x25),					MARCH_ALL,	O_RT|O_RS|MO_DELAYRT },
	{ "lwr",	"t,i(s)",	MIPS_OP(0x26),					MARCH_ALL,	O_RT|O_I16|O_RS|MO_DELAYRT|MO_IGNORERTD },
	{ "lwr",	"t,(s)",	MIPS_OP(0x26),					MARCH_ALL,	O_RT|O_RS|MO_DELAYRT|MO_IGNORERTD },
	{ "sb",		"t,i(s)",	MIPS_OP(0x28),					MARCH_ALL,	O_RT|O_I16|O_RS },
	{ "sb",		"t,(s)",	MIPS_OP(0x28),					MARCH_ALL,	O_RT|O_RS },
	{ "sh",		"t,i(s)",	MIPS_OP(0x29),					MARCH_ALL,	O_RT|O_I16|O_RS },
	{ "sh",		"t,(s)",	MIPS_OP(0x29),					MARCH_ALL,	O_RT|O_RS },
	{ "swl",	"t,i(s)",	MIPS_OP(0x2A),					MARCH_ALL,	O_RT|O_I16|O_RS },
	{ "swl",	"t,(s)",	MIPS_OP(0x2A),					MARCH_ALL,	O_RT|O_RS },
	{ "sw",		"t,i(s)",	MIPS_OP(0x2B),					MARCH_ALL,	O_RT|O_I16|O_RS },
	{ "sw",		"t,(s)",	MIPS_OP(0x2B),					MARCH_ALL,	O_RT|O_RS },
	{ "swr",	"t,i(s)",	MIPS_OP(0x2E),					MARCH_ALL,	O_RT|O_I16|O_RS },
	{ "swr",	"t,(s)",	MIPS_OP(0x2E),					MARCH_ALL,	O_RT|O_RS },
	// todo: cache
	{ "ll",		"t,i(s)",	MIPS_OP(0x30),					MARCH_PSP,	O_RT|O_I16|O_RS|MO_DELAYRT },
	{ "ll",		"t,(s)",	MIPS_OP(0x30),					MARCH_PSP,	O_RT|O_RS|MO_DELAYRT },
	{ "lwc1",	"T,i(s)",	MIPS_OP(0x31),					MARCH_PSP,	MO_FRT|O_I16|O_RS },
	{ "lwc1",	"T,(s)",	MIPS_OP(0x31),					MARCH_PSP,	MO_FRT|O_RS },
	{ "sc",		"t,i(s)",	MIPS_OP(0x38),					MARCH_PSP,	O_RT|O_I16|O_RS },
	{ "sc",		"t,(s)",	MIPS_OP(0x38),					MARCH_PSP,	O_RT|O_RS },
	{ "swc1",	"T,i(s)",	MIPS_OP(0x39),					MARCH_PSP,	MO_FRT|O_I16|O_RS },
	{ "swc1",	"T,(s)",	MIPS_OP(0x39),					MARCH_PSP,	MO_FRT|O_RS },


//     31---------26------------------------------------------5--------0
//     |=   SPECIAL|                                         | function|
//     ------6----------------------------------------------------6-----
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
// 000 | SLL   | ---   | SRL*1 | SRA   | SLLV  |  ---  | SRLV*2| SRAV  | 00..07
// 001 | JR    | JALR  | MOVZ  | MOVN  |SYSCALL| BREAK |  ---  | SYNC  | 08..0F
// 010 | MFHI  | MTHI  | MFLO  | MTLO  | DSLLV |  ---  |   *3  |  *4   | 10..17
// 011 | MULT  | MULTU | DIV   | DIVU  | MADD  | MADDU | ----  | ----- | 18..1F
// 100 | ADD   | ADDU  | SUB   | SUBU  | AND   | OR    | XOR   | NOR   | 20..27
// 101 | mfsa  | mtsa  | SLT   | SLTU  |  *5   |  *6   |  *7   |  *8   | 28..2F
// 110 | TGE   | TGEU  | TLT   | TLTU  | TEQ   |  ---  | TNE   |  ---  | 30..37
// 111 | dsll  |  ---  | dsrl  | dsra  |dsll32 |  ---  |dsrl32 |dsra32 | 38..3F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
// *1:	rotr when rs = 1 (PSP only)		*2:	rotrv when sa = 1 (PSP only)
// *3:	dsrlv on PS2, clz on PSP		*4:	dsrav on PS2, clo on PSP
// *5:	dadd on PS2, max on PSP			*6:	daddu on PS2, min on PSP
// *7:	dsub on PS2, msub on PSP		*8:	dsubu on PS2, msubu on PSP
	{ "sll",	"d,t,a",	MIPS_SPECIAL(0x00),				MARCH_ALL,	O_RD|O_RT|O_I5 },
	{ "sll",	"d,a",		MIPS_SPECIAL(0x00),				MARCH_ALL,	O_RDT|O_I5 },
	{ "nop",	"",			MIPS_SPECIAL(0x00),				MARCH_ALL,	0 },
	{ "srl",	"d,t,a",	MIPS_SPECIAL(0x02),				MARCH_ALL,	O_RD|O_RT|O_I5 },
	{ "srl",	"d,a",		MIPS_SPECIAL(0x02),				MARCH_ALL,	O_RDT|O_I5 },
	{ "rotr",	"d,t,a",	MIPS_SPECIAL(0x02)|MIPS_RS(1),	MARCH_PSP,	O_RD|O_RT|O_I5 },
	{ "rotr",	"d,a",		MIPS_SPECIAL(0x02)|MIPS_RS(1),	MARCH_PSP,	O_RDT|O_I5 },
	{ "sra",	"d,t,a",	MIPS_SPECIAL(0x03),				MARCH_ALL,	O_RD|O_RT|O_I5 },
	{ "sra",	"d,a",		MIPS_SPECIAL(0x03),				MARCH_ALL,	O_RDT|O_I5 },
	{ "sllv",	"d,t,s",	MIPS_SPECIAL(0x04),				MARCH_ALL,	O_RD|O_RT|O_RS },
	{ "sllv",	"d,s",		MIPS_SPECIAL(0x04),				MARCH_ALL,	O_RDT|O_RS },
	{ "srlv",	"d,t,s",	MIPS_SPECIAL(0x06),				MARCH_ALL,	O_RD|O_RT|O_RS },
	{ "srlv",	"d,s",		MIPS_SPECIAL(0x06),				MARCH_ALL,	O_RDT|O_RS },
	{ "rotrv",	"d,t,s",	MIPS_SPECIAL(0x06)|MIPS_SA(1),	MARCH_PSP,	O_RD|O_RT|O_RS },
	{ "rotrv",	"d,s",		MIPS_SPECIAL(0x06)|MIPS_SA(1),	MARCH_PSP,	O_RDT|O_RS },
	{ "srav",	"d,t,s",	MIPS_SPECIAL(0x07),				MARCH_ALL,	O_RD|O_RT|O_RS },
	{ "srav",	"d,s",		MIPS_SPECIAL(0x07),				MARCH_ALL,	O_RDT|O_RS },
	{ "jr",		"s",		MIPS_SPECIAL(0x08),				MARCH_ALL,	O_RS|MO_DELAY|MO_NODELAY },
	{ "jalr",	"s,d",		MIPS_SPECIAL(0x09),				MARCH_ALL,	O_RD|O_RS|MO_DELAY|MO_NODELAY },
	{ "jalr",	"s",		MIPS_SPECIAL(0x09)|MIPS_RD(31),	MARCH_ALL,	O_RS|MO_DELAY|MO_NODELAY },
	{ "movz",	"d,s,t",	MIPS_SPECIAL(0x0A),				MARCH_LV2,	O_RD|O_RT|O_RS },
	{ "movn",	"d,s,t",	MIPS_SPECIAL(0x0B),				MARCH_LV2,	O_RD|O_RT|O_RS },
	{ "syscall","b",		MIPS_SPECIAL(0x0C),				MARCH_ALL,	O_I20|MO_NODELAY },
	{ "break",	"b",		MIPS_SPECIAL(0x0D),				MARCH_ALL,	O_I20|MO_NODELAY },
	{ "sync",	"",			MIPS_SPECIAL(0x0F),				MARCH_LV2,	0 },
	{ "mfhi",	"d",		MIPS_SPECIAL(0x10),				MARCH_ALL,	O_RD },
	{ "mthi",	"s",		MIPS_SPECIAL(0x11),				MARCH_ALL,	O_RS },
	{ "mflo",	"d",		MIPS_SPECIAL(0x12),				MARCH_ALL,	O_RD },
	{ "mtlo",	"s",		MIPS_SPECIAL(0x13),				MARCH_ALL,	O_RS },
	{ "dsllv",	"d,t,s",	MIPS_SPECIAL(0x14),				MARCH_PS2,	O_RD|O_RT|O_RS },
	{ "dsllv",	"d,s",		MIPS_SPECIAL(0x14),				MARCH_PS2,	O_RT|O_RS },
	{ "dsrlv",	"d,t,s",	MIPS_SPECIAL(0x16),				MARCH_PS2,	O_RD|O_RT|O_RS },
	{ "dsrlv",	"d,s",		MIPS_SPECIAL(0x16),				MARCH_PS2,	O_RDT|O_RS },
	{ "clz",	"d,s",		MIPS_SPECIAL(0x16),				MARCH_PSP,	O_RD|O_RS },
	{ "dsrav",	"d,t,s",	MIPS_SPECIAL(0x17),				MARCH_PS2,	O_RD|O_RT|O_RS },
	{ "dsrav",	"d,s",		MIPS_SPECIAL(0x17),				MARCH_PS2,	O_RDT|O_RS },
	{ "clo",	"d,s",		MIPS_SPECIAL(0x17),				MARCH_PSP,	O_RD|O_RS },
	{ "mult",	"s,t",		MIPS_SPECIAL(0x18),				MARCH_ALL,	O_RS|O_RT },
	{ "mult",	"r\x0,s,t",	MIPS_SPECIAL(0x18),				MARCH_ALL,	O_RS|O_RT },
	{ "multu",	"s,t",		MIPS_SPECIAL(0x19),				MARCH_ALL,	O_RS|O_RT },
	{ "multu",	"r\x0,s,t",	MIPS_SPECIAL(0x19),				MARCH_ALL,	O_RS|O_RT },
	{ "div",	"s,t",		MIPS_SPECIAL(0x1A),				MARCH_ALL,	O_RS|O_RT },
	{ "div",	"r\x0,s,t",	MIPS_SPECIAL(0x1A),				MARCH_ALL,	O_RS|O_RT },
	{ "divu",	"s,t",		MIPS_SPECIAL(0x1B),				MARCH_ALL,	O_RS|O_RT },
	{ "divu",	"r\x0,s,t",	MIPS_SPECIAL(0x1B),				MARCH_ALL,	O_RS|O_RT },
	{ "madd",	"s,t",		MIPS_SPECIAL(0x1C),				MARCH_PSP,	O_RS|O_RT },
	{ "maddu",	"s,t",		MIPS_SPECIAL(0x1D),				MARCH_PSP,	O_RS|O_RT },
	{ "add",	"d,s,t",	MIPS_SPECIAL(0x20),				MARCH_ALL,	O_RD|O_RS|O_RT },
	{ "add",	"s,t",		MIPS_SPECIAL(0x20),				MARCH_ALL,	O_RSD|O_RT },
	{ "addu",	"d,s,t",	MIPS_SPECIAL(0x21),				MARCH_ALL,	O_RD|O_RS|O_RT },
	{ "addu",	"s,t",		MIPS_SPECIAL(0x21),				MARCH_ALL,	O_RSD|O_RT },
	{ "move",	"d,s",		MIPS_SPECIAL(0x21),				MARCH_ALL,	O_RD|O_RS },
	{ "sub",	"d,s,t",	MIPS_SPECIAL(0x22),				MARCH_ALL,	O_RD|O_RS|O_RT },
	{ "sub",	"s,t",		MIPS_SPECIAL(0x22),				MARCH_ALL,	O_RSD|O_RT },
	{ "neg",	"d,t",		MIPS_SPECIAL(0x22),				MARCH_ALL,	O_RD|O_RT },
	{ "subu",	"d,s,t",	MIPS_SPECIAL(0x23),				MARCH_ALL,	O_RD|O_RS|O_RT },
	{ "subu",	"s,t",		MIPS_SPECIAL(0x23),				MARCH_ALL,	O_RSD|O_RT },
	{ "negu",	"d,t",		MIPS_SPECIAL(0x23),				MARCH_ALL,	O_RD|O_RT },
	{ "and",	"d,s,t",	MIPS_SPECIAL(0x24),				MARCH_ALL,	O_RD|O_RS|O_RT },
	{ "and",	"s,t",		MIPS_SPECIAL(0x24),				MARCH_ALL,	O_RSD|O_RT },
	{ "or",		"d,s,t",	MIPS_SPECIAL(0x25),				MARCH_ALL,	O_RS|O_RT|O_RD },
	{ "or",		"s,t",		MIPS_SPECIAL(0x25),				MARCH_ALL,	O_RSD|O_RT },
	{ "xor",	"d,s,t",	MIPS_SPECIAL(0x26), 			MARCH_ALL,	O_RS|O_RD|O_RT },
	{ "eor",	"d,s,t",	MIPS_SPECIAL(0x26),				MARCH_ALL,	O_RS|O_RD|O_RT },
	{ "xor",	"s,t",		MIPS_SPECIAL(0x26), 			MARCH_ALL,	O_RSD|O_RT },
	{ "eor",	"s,t",		MIPS_SPECIAL(0x26), 			MARCH_ALL,	O_RSD|O_RT },
	{ "nor",	"d,s,t",	MIPS_SPECIAL(0x27),				MARCH_ALL,	O_RS|O_RT|O_RD },
	{ "nor",	"s,t",		MIPS_SPECIAL(0x27),				MARCH_ALL,	O_RSD|O_RT },
	{ "mfsa",	"d",		MIPS_SPECIAL(0x28),				MARCH_PS2,	O_RD },
	{ "mtsa",	"s",		MIPS_SPECIAL(0x29),				MARCH_PS2,	O_RS },
	{ "slt",	"d,s,t",	MIPS_SPECIAL(0x2A),				MARCH_ALL,	O_RD|O_RT|O_RS },
	{ "slt",	"s,t",		MIPS_SPECIAL(0x2A),				MARCH_ALL,	O_RSD|O_RT},
	{ "sltu",	"d,s,t",	MIPS_SPECIAL(0x2B),				MARCH_ALL,	O_RD|O_RT|O_RS },
	{ "sltu",	"s,t",		MIPS_SPECIAL(0x2B),				MARCH_ALL,	O_RSD|O_RT },
	{ "dadd",	"d,s,t",	MIPS_SPECIAL(0x2C),				MARCH_PS2,	O_RD|O_RT|O_RS },
	{ "max",	"d,s,t",	MIPS_SPECIAL(0x2C),				MARCH_PSP,	O_RD|O_RT|O_RS },
	{ "daddu",	"d,s,t",	MIPS_SPECIAL(0x2D), 			MARCH_PS2,	O_RD|O_RT|O_RS },
	{ "min",	"d,s,t",	MIPS_SPECIAL(0x2D), 			MARCH_PSP,	O_RD|O_RT|O_RS },
	{ "dsub",	"d,s,t",	MIPS_SPECIAL(0x2E), 			MARCH_PS2,	O_RD|O_RT|O_RS },
	{ "msub",	"s,t",		MIPS_SPECIAL(0x2E),				MARCH_PSP,	O_RS|O_RT },
	{ "dsubu",	"d,s,t",	MIPS_SPECIAL(0x2F), 			MARCH_PS2,	O_RD|O_RT|O_RS },
	{ "msubu",	"s,t",		MIPS_SPECIAL(0x2F),				MARCH_PSP,	O_RS|O_RT },
	{ "tge",	"s,t",		MIPS_SPECIAL(0x30),				MARCH_LV2,	O_RSD|O_RT },
	{ "tgeu",	"s,t",		MIPS_SPECIAL(0x31),				MARCH_LV2,	O_RSD|O_RT },
	{ "tlt",	"s,t",		MIPS_SPECIAL(0x32),				MARCH_LV2,	O_RSD|O_RT },
	{ "tltu",	"s,t",		MIPS_SPECIAL(0x33),				MARCH_LV2,	O_RSD|O_RT },
	{ "teq",	"s,t",		MIPS_SPECIAL(0x34),				MARCH_LV2,	O_RSD|O_RT },
	{ "tne",	"s,t",		MIPS_SPECIAL(0x36),				MARCH_LV2,	O_RSD|O_RT },
	{ "dsll",	"d,t,a",	MIPS_SPECIAL(0x38),				MARCH_PS2,	O_RD|O_RT|O_I5 },
	{ "dsll",	"d,a",		MIPS_SPECIAL(0x38),				MARCH_PS2,	O_RDT|O_I5 },
	{ "dsrl",	"d,t,a",	MIPS_SPECIAL(0x3A),				MARCH_PS2,	O_RD|O_RT|O_I5 },
	{ "dsrl",	"d,a",		MIPS_SPECIAL(0x3A),				MARCH_PS2,	O_RDT|O_I5 },
	{ "dsra",	"d,t,a",	MIPS_SPECIAL(0x3B),				MARCH_PS2,	O_RD|O_RT|O_I5 },
	{ "dsra",	"d,a",		MIPS_SPECIAL(0x3B),				MARCH_PS2,	O_RDT|O_I5 },
	{ "dsll32",	"d,t,a",	MIPS_SPECIAL(0x3C),				MARCH_PS2,	O_RD|O_RT|O_I5 },
	{ "dsll32",	"d,a",		MIPS_SPECIAL(0x3C),				MARCH_PS2,	O_RDT|O_I5 },
	{ "dsrl32",	"d,t,a",	MIPS_SPECIAL(0x3E),				MARCH_PS2,	O_RD|O_RT|O_I5 },
	{ "dsrl32",	"d,a",		MIPS_SPECIAL(0x3E),				MARCH_PS2,	O_RDT|O_I5 },
	{ "dsra32",	"d,t,a",	MIPS_SPECIAL(0x3F),				MARCH_PS2,	O_RD|O_RT|O_I5 },
	{ "dsra32",	"d,a",		MIPS_SPECIAL(0x3F),				MARCH_PS2,	O_RDT|O_I5 },

//     REGIMM: encoded by the rt field when opcode field = REGIMM.
//     31---------26----------20-------16------------------------------0
//     |=    REGIMM|          |   rt    |                              |
//     ------6---------------------5------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 | BLTZ  | BGEZ  | BLTZL | BGEZL |  ---  |  ---  |  ---  |  ---  | 00-07
//  01 | tgei  | tgeiu | tlti  | tltiu | teqi  |  ---  | tnei  |  ---  | 08-0F
//  10 | BLTZAL| BGEZAL|BLTZALL|BGEZALL|  ---  |  ---  |  ---  |  ---  | 10-17
//  11 | mtsab | mtsah |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 18-1F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "bltz",	"s,I",		MIPS_REGIMM(0x00),				MARCH_ALL,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bgez",	"s,I",		MIPS_REGIMM(0x01),				MARCH_ALL,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bltzl",	"s,I",		MIPS_REGIMM(0x02),				MARCH_LV2,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bgezl",	"s,I",		MIPS_REGIMM(0x03),				MARCH_LV2,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "tgei",	"s,I",		MIPS_REGIMM(0x08),				MARCH_PS2,	O_RS|O_I16 },
	{ "tgeiu",	"s,I",		MIPS_REGIMM(0x09),				MARCH_PS2,	O_RS|O_I16 },
	{ "tlti",	"s,I",		MIPS_REGIMM(0x0A),				MARCH_PS2,	O_RS|O_I16 },
	{ "tltiu",	"s,I",		MIPS_REGIMM(0x0B),				MARCH_PS2,	O_RS|O_I16 },
	{ "teqi",	"s,I",		MIPS_REGIMM(0x0C),				MARCH_PS2,	O_RS|O_I16 },
	{ "tnei",	"s,I",		MIPS_REGIMM(0x0E),				MARCH_PS2,	O_RS|O_I16 },
	{ "bltzal",	"s,I",		MIPS_REGIMM(0x10),				MARCH_ALL,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bgezal",	"s,I",		MIPS_REGIMM(0x11),				MARCH_ALL,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bltzall","s,I",		MIPS_REGIMM(0x12),				MARCH_LV2,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bgezall","s,I",		MIPS_REGIMM(0x13),				MARCH_LV2,	O_RS|O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "mtsab",	"s,I",		MIPS_REGIMM(0x18),				MARCH_PS2,	O_RS|O_I16 },
	{ "mtsah",	"s,I",		MIPS_REGIMM(0x19),				MARCH_PS2,	O_RS|O_I16 },

//     31-------26------21---------------------------------------------0
//     |=    COP1|  rs  |                                              |
//     -----6-------5---------------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 |  MFC1 |  ---  |  CFC1 |  ---  |  MTC1 |  ---  |  CTC1 |  ---  | 00..07
//  01 |  BC*  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 08..0F
//  10 |  S*   |  ---  |  ---  |  ---  |  W*   |  ---  |  ---  |  ---  | 10..17
//  11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 18..1F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "mfc1",	"t,S",		MIPS_COP1(0x00),				MARCH_LV2,	O_RT|MO_FRS },
	{ "cfc1",	"t,S",		MIPS_COP1(0x02),				MARCH_LV2,	O_RT|MO_FRS },
	{ "mtc1",	"t,S",		MIPS_COP1(0x04),				MARCH_LV2,	O_RT|MO_FRS },
	{ "ctc1",	"t,S",		MIPS_COP1(0x06),				MARCH_LV2,	O_RT|MO_FRS },

//     31---------21-------16------------------------------------------0
//     |=    COP1BC|  rt   |                                           |
//     ------11---------5-----------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 |  BC1F | BC1T  | BC1FL | BC1TL |  ---  |  ---  |  ---  |  ---  | 00..07
//  01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 08..0F
//  10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 10..17
//  11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 18..1F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "bc1f",	"I",		MIPS_COP1BC(0x00),				MARCH_LV2,	O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bc1t",	"I",		MIPS_COP1BC(0x01),				MARCH_LV2,	O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bc1fl",	"I",		MIPS_COP1BC(0x02),				MARCH_LV2,	O_I16|O_IPCR|MO_DELAY|MO_NODELAY },
	{ "bc1tl",	"I",		MIPS_COP1BC(0x03),				MARCH_LV2,	O_I16|O_IPCR|MO_DELAY|MO_NODELAY },

//     31---------21------------------------------------------5--------0
//     |=  COP1S  |                                          | function|
//     -----11----------------------------------------------------6-----
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
// 000 |  add  |  sub  |  mul  |  div  | sqrt  |  abs  |  mov  |  neg  | 00..07
// 001 |  ---  |  ---  |  ---  |  ---  |round.w|trunc.w|ceil.w |floor.w| 08..0F
// 010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | rsqrt |  ---  | 10..17
// 011 |  adda |  suba | mula  |  ---  | madd  |  msub | madda | msuba | 18..1F
// 100 |  ---  |  ---  |  ---  |  ---  | cvt.w |  ---  |  ---  |  ---  | 20..27
// 101 |  max  |  min  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 28..2F
// 110 |  c.f  | c.un  | c.eq  | c.ueq |c.(o)lt| c.ult |c.(o)le| c.ule | 30..37
// 110 |  c.sf | c.ngle| c.seq | c.ngl | c.lt  | c.nge | c.le  | c.ngt | 38..3F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "add.s",		"D,S,T",	MIPS_COP1S(0x00),			MARCH_LV2,	MO_FRT|MO_FRD|MO_FRS },
	{ "add.s",		"S,T",		MIPS_COP1S(0x00),			MARCH_LV2,	MO_FRT|MO_FRSD },
	{ "sub.s",		"D,S,T",	MIPS_COP1S(0x01),			MARCH_LV2,	MO_FRT|MO_FRD|MO_FRS },
	{ "sub.s",		"S,T",		MIPS_COP1S(0x01),			MARCH_LV2,	MO_FRT|MO_FRSD },
	{ "mul.s",		"D,S,T",	MIPS_COP1S(0x02),			MARCH_LV2,	MO_FRT|MO_FRD|MO_FRS },
	{ "mul.s",		"S,T",		MIPS_COP1S(0x02),			MARCH_LV2,	MO_FRT|MO_FRSD },
	{ "div.s",		"D,S,T",	MIPS_COP1S(0x03),			MARCH_LV2,	MO_FRT|MO_FRD|MO_FRS },
	{ "div.s",		"S,T",		MIPS_COP1S(0x03),			MARCH_LV2,	MO_FRT|MO_FRSD },
	{ "sqrt.s",		"D,S",		MIPS_COP1S(0x04),			MARCH_LV2,	MO_FRD|MO_FRS },
	{ "abs.s",		"D,S",		MIPS_COP1S(0x05),			MARCH_LV2,	MO_FRD|MO_FRS },
	{ "mov.s",		"D,S",		MIPS_COP1S(0x06),			MARCH_LV2,	MO_FRD|MO_FRS },
	{ "neg.s",		"D,S",		MIPS_COP1S(0x07),			MARCH_LV2,	MO_FRD|MO_FRS },
	{ "round.w.s",	"D,S",		MIPS_COP1S(0x0C),			MARCH_PSP,	MO_FRD|MO_FRS },
	{ "trunc.w.s",	"D,S",		MIPS_COP1S(0x0D),			MARCH_PSP,	MO_FRD|MO_FRS },
	{ "ceil.w.s",	"D,S",		MIPS_COP1S(0x0E),			MARCH_PSP,	MO_FRD|MO_FRS },
	{ "floor.w.s",	"D,S",		MIPS_COP1S(0x0F),			MARCH_PSP,	MO_FRD|MO_FRS },
	{ "rsqrt.w.s",	"D,S",		MIPS_COP1S(0x16),			MARCH_PS2,	MO_FRD|MO_FRS },
	{ "adda.s",		"S,T",		MIPS_COP1S(0x18),			MARCH_PS2,	MO_FRT|MO_FRS },
	{ "suba.s",		"S,T",		MIPS_COP1S(0x19),			MARCH_PS2,	MO_FRT|MO_FRS },
	{ "mula.s",		"S,T",		MIPS_COP1S(0x1A),			MARCH_PS2,	MO_FRT|MO_FRS },
	{ "madd.s",		"D,S,T",	MIPS_COP1S(0x1C),			MARCH_PS2,	MO_FRT|MO_FRD|MO_FRS },
	{ "madd.s",		"S,T",		MIPS_COP1S(0x1C),			MARCH_PS2,	MO_FRT|MO_FRSD },
	{ "msub.s",		"D,S,T",	MIPS_COP1S(0x1D),			MARCH_PS2,	MO_FRT|MO_FRD|MO_FRS },
	{ "msub.s",		"S,T",		MIPS_COP1S(0x1D),			MARCH_PS2,	MO_FRT|MO_FRSD },
	{ "madda.s",	"S,T",		MIPS_COP1S(0x1E),			MARCH_PS2,	MO_FRT|MO_FRS },
	{ "msuba.s",	"S,T",		MIPS_COP1S(0x1F),			MARCH_PS2,	MO_FRT|MO_FRS },
	{ "cvt.w.s",	"D,S",		MIPS_COP1S(0x24),			MARCH_LV2,	MO_FRD|MO_FRS },
	{ "max.s",		"D,S,T",	MIPS_COP1S(0x28),			MARCH_PS2,	MO_FRT|MO_FRD|MO_FRS },
	{ "max.s",		"S,T",		MIPS_COP1S(0x28),			MARCH_PS2,	MO_FRT|MO_FRSD },
	{ "min.s",		"D,S,T",	MIPS_COP1S(0x29),			MARCH_PS2,	MO_FRT|MO_FRD|MO_FRS },
	{ "min.s",		"S,T",		MIPS_COP1S(0x29),			MARCH_PS2,	MO_FRT|MO_FRSD },
	{ "c.f.s",		"S,T",		MIPS_COP1S(0x30),			MARCH_LV2,	MO_FRT|MO_FRS },
	{ "c.un.s",		"S,T",		MIPS_COP1S(0x31),			MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.eq.s",		"S,T",		MIPS_COP1S(0x32),			MARCH_LV2,	MO_FRT|MO_FRS },
	{ "c.ueq.s",	"S,T",		MIPS_COP1S(0x33),			MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.olt.s",	"S,T",		MIPS_COP1S(0x34),			MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.lt.s",		"S,T",		MIPS_COP1S(0x34),			MARCH_PS2,	MO_FRT|MO_FRS },
	{ "c.ult.s",	"S,T",		MIPS_COP1S(0x35),			MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.ole.s",	"S,T",		MIPS_COP1S(0x36),			MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.le.s",		"S,T",		MIPS_COP1S(0x36),			MARCH_PS2,	MO_FRT|MO_FRS },
	{ "c.ule.s",	"S,T",		MIPS_COP1S(0x37),			MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.sf.s",		"S,T",		MIPS_COP1S(0x38),			MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.ngle.s",	"S,T",		MIPS_COP1S(0x39),			MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.seq.s",	"S,T",		MIPS_COP1S(0x3A),			MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.ngl.s",	"S,T",		MIPS_COP1S(0x3B),			MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.lt.s",		"S,T",		MIPS_COP1S(0x3C),			MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.nge.s",	"S,T",		MIPS_COP1S(0x3D),			MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.le.s",		"S,T",		MIPS_COP1S(0x3E),			MARCH_PSP,	MO_FRT|MO_FRS },
	{ "c.ngt.s",	"S,T",		MIPS_COP1S(0x3F),			MARCH_PSP,	MO_FRT|MO_FRS },

//     COP1W: encoded by function field
//     31---------21------------------------------------------5--------0
//     |=  COP1W  |                                          | function|
//     -----11----------------------------------------------------6-----
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
// 000 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 00..07
// 001 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 08..0F
// 010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 10..17
// 011 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 18..1F
// 100 |cvt.s.w|  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 20..27
// 101 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 28..2F
// 110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 30..37
// 110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 38..3F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "cvt.s.w",	"D,S",		MIPS_COP1W(0x20),			MARCH_LV2,	MO_FRD|MO_FRS },




	// END
	{ NULL,		NULL,		0,			0 }
};