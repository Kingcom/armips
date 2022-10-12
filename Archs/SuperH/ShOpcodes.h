#pragma once

#include <cstdint>

#define SH_SUPERH1 (1 << 0)
#define SH_SUPERH2 (1 << 1)

#define SH_IMM16 	   (1 << 0)
#define SH_IMM32 	   (1 << 1)
#define SH_IMMREL      (1 << 2)
#define SH_FREG		   (1 << 3)
#define SH_IMMSIGNED   (1 << 4)
#define SH_DELAYED	   (1 << 5)
#define SH_PCRELMANUAL (1 << 6)

#define SH_MUSTBEALIGNED (SH_IMM16 | SH_IMM32)

enum ShInstructionFormat
{
	// x - op code
	// m - source
	// n - dest
	// i - immediate data
	// d - displacement
	             // 15                0
	SHFMT_0 = 0, // xxxx xxxx xxxx xxxx
	SHFMT_N,     // xxxx nnnn xxxx xxxx
	SHFMT_M,     // xxxx mmmm xxxx xxxx
	SHFMT_NM,    // xxxx nnnn mmmm xxxx
	SHFMT_MD,    // xxxx xxxx mmmm dddd
	SHFMT_ND4,   // xxxx xxxx nnnn dddd
	SHFMT_NMD,   // xxxx nnnn mmmm dddd
	SHFMT_D,     // xxxx xxxx dddd dddd
	SHFMT_D12,   // xxxx dddd dddd dddd
	SHFMT_ND8,   // xxxx nnnn dddd dddd
	SHFMT_I,     // xxxx xxxx iiii iiii
	SHFMT_NI,    // xxxx nnnn iiii iiii
	SHFMT_INVALID
};

struct ShArchDefinition
{
	const char* name;
	int supportSets;
	int excludeMask;
	int flags;
};

extern const ShArchDefinition shArchs[];

struct tShOpcode
{
	const char* name;
	const char* encoding;
	uint16_t base;
	int format;
	int archs;
	int flags;
};

extern const tShOpcode shOpcodes[];
