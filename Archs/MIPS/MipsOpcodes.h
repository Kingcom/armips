#pragma once
#include "Mips.h"

#define O_RS			0x00000001	// source reg
#define O_RT			0x00000002	// target reg
#define O_RD			0x00000004	// dest reg
#define O_I16			0x00000008	// 16 bit immediate
#define O_I20			0x00000010	// 16 bit immediate
#define O_IPCA			0x00000020	// pc >> 2
#define O_IPCR			0x00000080	// PC, -> difference >> 2
#define O_I26			0x00000200	// 26 bit immediate
#define O_I5			0x00000400	// 5 bit immediate
#define O_RSD			(0x00000800|O_RS|O_RD)	// rs = rd
#define O_RST			(0x00001000|O_RS|O_RT)	// rs = rt
#define O_RDT			(0x00002000|O_RD|O_RT)	// rd = rt
#define MO_DELAY		0x00004000	// delay slot follows
#define MO_NODELAY		0x00008000	// can't be in a delay slot
#define MO_DELAYRT		0x00010000	// rt won't be available for one instruction
#define MO_IGNORERTD	0x00020000	// don't care for rt delay

#define MO_FRS			0x00040000	// float rs
#define MO_FRD			0x00080000	// float rd
#define MO_FRT			0x00100000	// float rt
#define MO_FRSD			(0x00200000|MO_FRS|MO_FRD)	// float rs + rd
#define MO_FRST			(0x00400000|MO_FRS|MO_FRT)	// float rs + rt
#define MO_FRDT			(0x00800000|MO_FRD|MO_FRT)	// float rt + rd


#define MIPS1			0x0000001
#define MIPS2			0x0000002
#define MIPS3			0x0000003



#define BITFIELD(START,LENGTH,VALUE)	(((VALUE) << (START)))
#define MIPS_FUNC(VALUE)				BITFIELD(0,6,(VALUE))
#define MIPS_SA(VALUE)					BITFIELD(6,5,(VALUE))
#define MIPS_SECFUNC(VALUE)				MIPS_SA((VALUE))
#define MIPS_OP(VALUE)					BITFIELD(26,6,(VALUE))

#define MIPS_RS(VALUE)					BITFIELD(21,5,(VALUE))
#define MIPS_RT(VALUE)					BITFIELD(16,5,(VALUE))
#define MIPS_RD(VALUE)					BITFIELD(11,5,(VALUE))
#define MIPS_FS(VALUE)					MIPS_RD((VALUE))
#define MIPS_FT(VALUE)					MIPS_RT((VALUE))
#define MIPS_FD(VALUE)					MIPS_SA((VALUE))

#define MIPS_SPECIAL(VALUE)				(MIPS_OP(0) | MIPS_FUNC(VALUE))
#define MIPS_REGIMM(VALUE)				(MIPS_OP(1) | MIPS_RT(VALUE))
#define MIPS_COP0(VALUE)				(MIPS_OP(16) | MIPS_RS(VALUE))
#define MIPS_COP1(VALUE)				(MIPS_OP(17) | MIPS_RS(VALUE))
#define MIPS_COP1BC(VALUE)				(MIPS_COP1(8) | MIPS_RT(VALUE))
#define MIPS_COP1S(VALUE)				(MIPS_COP1(16) | MIPS_FUNC(VALUE))
#define MIPS_COP1W(VALUE)				(MIPS_COP1(20) | MIPS_FUNC(VALUE))

typedef struct {
	char* name;
	char* encoding;
	int destencoding;
	int ver:4;
	int flags:28;
} tMipsOpcode;

extern const tMipsOpcode MipsOpcodes[];
