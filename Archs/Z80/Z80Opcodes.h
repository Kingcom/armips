#pragma once

#define Z80_PARAM_NONE			0x00
#define Z80_PARAM_A				0x01	// a
#define Z80_PARAM_MEMBC_MEMDE	0x02	// (bc), (de)
#define Z80_PARAM_REG8_MEMHL	0x03	// b, c, d, e, h, l, (hl), a
#define Z80_PARAM_REG16_SP		0x04	// bc, de, hl, sp
#define Z80_PARAM_REG16_AF		0x05	// bc, de, hl, af
#define Z80_PARAM_HL			0x06	// hl
#define Z80_PARAM_MEMHL			0x07	// (hl)
#define Z80_PARAM_HLI_HLD		0x08	// hli, hld, hl+, hl-
#define Z80_PARAM_SP			0x09	// sp
#define Z80_PARAM_IMMEDIATE		0x0A	// imm
#define	Z80_PARAM_MEMIMMEDIATE	0x0B	// (imm)
#define Z80_PARAM_FF00_C		0x0C	// (0xFF00+c)
#define Z80_PARAM_SP_IMM		0x0D	// sp+s8
#define Z80_PARAM_CONDITION		0x0E	// nz, z, nc, c
#define Z80_PARAM_AF			0x0F	// af
#define Z80_PARAM_AF_PRIME		0x10	// af'
#define Z80_PARAM_MEMSP			0x11	// (sp)
#define Z80_PARAM_DE			0x12	// de
#define Z80_PARAM_REG8			0x13	// b, c, d, e, h, l, a
#define Z80_PARAM_MEMC			0x14	// (c)
#define Z80_PARAM_BC_DE_SP		0x15	// bc, de, sp
#define Z80_PARAM_IR			0x16	// i, r
#define Z80_PARAM_IX_IY			0x17	// ix, iy
#define Z80_PARAM_MEMIX_MEMIY	0x18	// (ix+imm), (iy+imm)
#define	Z80_PARAM_REG16_IX_IY	0x19	// bc, de, ix, iy, sp

#define Z80_REG8_B				0x00	// b
#define Z80_REG8_C				0x01	// c
#define Z80_REG8_D				0x02	// d
#define Z80_REG8_E				0x03	// e
#define Z80_REG8_H				0x04	// h
#define Z80_REG8_L				0x05	// l
#define Z80_REG8_MEMHL			0x06	// (hl)
#define Z80_REG8_A				0x07	// a
#define Z80_REG8_BIT_ALL		( Z80_REG_BIT(Z80_REG8_B) | Z80_REG_BIT(Z80_REG8_C) \
								| Z80_REG_BIT(Z80_REG8_D) | Z80_REG_BIT(Z80_REG8_E) \
								| Z80_REG_BIT(Z80_REG8_H) | Z80_REG_BIT(Z80_REG8_L) \
								| Z80_REG_BIT(Z80_REG8_A) )

#define Z80_REG16_BC			0x00	// bc
#define Z80_REG16_DE			0x01	// de
#define Z80_REG16_HL			0x02	// hl
#define Z80_REG16_SP			0x03	// sp
#define Z80_REG16_AF			0x03	// af (yes, same as sp)
#define Z80_REG16_IX			0x02	// ix (yes, same as hl)
#define Z80_REG16_IY			0x82	// iy
#define Z80_REG16_BIT_ALL		( Z80_REG_BIT(Z80_REG16_BC) | Z80_REG_BIT(Z80_REG16_DE) \
								| Z80_REG_BIT(Z80_REG16_HL) | Z80_REG_BIT(Z80_REG16_SP) \
								| Z80_REG_BIT(Z80_REG16_AF) | Z80_REG_BIT(Z80_REG16_IX) \
								| Z80_REG_BIT(Z80_REG16_IY) )

#define Z80_COND_NZ				0x00	// nz
#define Z80_COND_Z				0x01	// z
#define Z80_COND_NC				0x02	// nc
#define Z80_COND_C				0x03	// c
#define Z80_COND_PO				0x04	// po
#define Z80_COND_PE				0x05	// pe
#define Z80_COND_P				0x06	// p
#define Z80_COND_M				0x07	// m
#define Z80_COND_BIT_ALL		( Z80_REG_BIT(Z80_COND_NZ) | Z80_REG_BIT(Z80_COND_Z)  \
								| Z80_REG_BIT(Z80_COND_NC) | Z80_REG_BIT(Z80_COND_C)  \
								| Z80_REG_BIT(Z80_COND_PO) | Z80_REG_BIT(Z80_COND_PE) \
								| Z80_REG_BIT(Z80_COND_P)  | Z80_REG_BIT(Z80_COND_M)  )
#define Z80_COND_BIT_GB			( Z80_REG_BIT(Z80_COND_NZ) | Z80_REG_BIT(Z80_COND_Z)  \
								| Z80_REG_BIT(Z80_COND_NC) | Z80_REG_BIT(Z80_COND_C)  )

#define Z80_REG_I				0x00	// i
#define Z80_REG_R				0x01	// r
#define Z80_REG_IR_ALL			( Z80_REG_BIT(Z80_REG_I) | Z80_REG_BIT(Z80_REG_R) )

#define Z80_REG_BIT_ALL			0xFFFFFFFF

#define Z80_PREFIX_CB			0x00000001
#define Z80_IMMEDIATE_U3		0x00000002
#define Z80_IMMEDIATE_U8		0x00000004
#define Z80_IMMEDIATE_S8		0x00000008
#define Z80_IMMEDIATE_U16		0x00000010
#define Z80_STOP				0x00000020
#define Z80_RST					0x00000040
#define Z80_LOAD_REG8_REG8		0x00000080
#define Z80_ADD_SUB_IMMEDIATE	0x00000100
#define Z80_NEGATE_IMM			0x00000200
#define Z80_JUMP_RELATIVE		0x00000400
#define Z80_GAMEBOY				0x00000800
#define Z80_Z80					0x00001000
#define Z80_PREFIX_ED			0x00002000
#define Z80_INTERRUPT_MODE		0x00004000
#define Z80_PREFIX_IX_IY		0x00008000
#define Z80_IMMEDIATE2_U8		0x00010000

#define Z80_HAS_IMMEDIATE		( Z80_IMMEDIATE_U3 | Z80_IMMEDIATE_U8 | Z80_IMMEDIATE_S8 \
								| Z80_IMMEDIATE_U16 | Z80_RST | Z80_JUMP_RELATIVE \
								| Z80_INTERRUPT_MODE | Z80_IMMEDIATE2_U8 )
#define Z80_HAS_2_IMMEDIATES	( Z80_IMMEDIATE2_U8 )
#define Z80_IS_PARAM_IX_IY(x)	( x == Z80_PARAM_IX_IY || x == Z80_PARAM_REG16_IX_IY \
								|| x == Z80_PARAM_MEMIX_MEMIY )

#define Z80_REG_BIT(reg) (1 << (reg & 0xF))

struct tZ80Opcode
{
	const wchar_t* name;
	unsigned char length;
	unsigned char encoding;
	unsigned char lhs;
	unsigned char rhs;
	char lhsShift : 4;
	char rhsShift : 4;
	int flags;
};

extern const tZ80Opcode Z80Opcodes[];
