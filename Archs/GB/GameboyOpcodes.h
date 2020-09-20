#pragma once

#define GB_PARAM_NONE			0x00
#define GB_PARAM_A				0x01	// a
#define GB_PARAM_MEMBC_MEMDE	0x02	// (bc), (de)
#define GB_PARAM_REG8_MEMHL		0x03	// b, c, d, e, h, l, (hl), a
#define GB_PARAM_REG16_SP		0x04	// bc, de, hl, sp
#define GB_PARAM_REG16_AF		0x05	// bc, de, hl, af
#define GB_PARAM_HL				0x06	// hl
#define GB_PARAM_MEMHL			0x07	// (hl)
#define GB_PARAM_HLI_HLD		0x08	// hli, hld, hl+, hl-
#define GB_PARAM_SP				0x09	// sp
#define GB_PARAM_IMMEDIATE		0x0A	// imm
#define	GB_PARAM_MEMIMMEDIATE	0x0B	// (imm)
#define GB_PARAM_FF00_C			0x0C	// (0xFF00+c)
#define GB_PARAM_SP_IMM			0x0D	// sp+s8

#define GB_REG8_B				0x00	// b
#define GB_REG8_C				0x01	// c
#define GB_REG8_D				0x02	// d
#define GB_REG8_E				0x03	// e
#define GB_REG8_H				0x04	// h
#define GB_REG8_L				0x05	// l
#define GB_REG8_MEMHL			0x06	// (hl)
#define GB_REG8_A				0x07	// a
#define GB_REG8_BIT_ALL			( GB_REG_BIT(GB_REG8_B) | GB_REG_BIT(GB_REG8_C) \
								| GB_REG_BIT(GB_REG8_D) | GB_REG_BIT(GB_REG8_E) \
								| GB_REG_BIT(GB_REG8_H) | GB_REG_BIT(GB_REG8_L) \
								| GB_REG_BIT(GB_REG8_A) )

#define GB_REG16_BC				0x00	// bc
#define GB_REG16_DE				0x01	// de
#define GB_REG16_HL				0x02	// hl
#define GB_REG16_SP				0x03	// sp
#define GB_REG16_AF				0x03	// af (yes, same as sp)
#define GB_REG16_BIT_ALL		( GB_REG_BIT(GB_REG16_BC) | GB_REG_BIT(GB_REG16_DE) \
								| GB_REG_BIT(GB_REG16_HL) | GB_REG_BIT(GB_REG16_SP) \
								| GB_REG_BIT(GB_REG16_AF) )

#define GB_REG_BIT_ALL			0xFFFFFFFF

#define GB_PREFIX				0x00000001
#define GB_IMMEDIATE_U8			0x00000002
#define GB_IMMEDIATE_S8			0x00000004
#define GB_IMMEDIATE_U16		0x00000008
#define GB_STOP					0x00000010
#define GB_LOAD_REG8_REG8		0x00000020
#define GB_ADD_IMMEDIATE		0x00000040
#define GB_SUB_IMMEDIATE		0x00000080
#define GB_NEGATE_IMM			0x00000100

#define GB_REG_BIT(reg) (1 << reg)

struct tGameboyOpcode
{
	const wchar_t* name;
	unsigned char length;
	unsigned char encoding;
	unsigned char lhs : 4;
	unsigned char rhs : 4;
	char lhsShift : 4;
	char rhsShift : 4;
	int flags;
};

extern const tGameboyOpcode GameboyOpcodes[];
