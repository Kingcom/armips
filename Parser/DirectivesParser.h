#pragma once

#include <memory>
#include <unordered_map>

class CAssemblerCommand;
class Parser;

using DirectiveFunc = std::unique_ptr<CAssemblerCommand> (*)(Parser&,int);

struct DirectiveEntry {
	DirectiveFunc function;
	int flags;
};

using DirectiveMap = std::unordered_multimap<std::wstring, const DirectiveEntry>;

#define DIRECTIVE_USERMASK			0x0000FFFF

// Global flags
#define DIRECTIVE_NOCASHON			0x00010000
#define DIRECTIVE_NOCASHOFF			0x00020000
#define DIRECTIVE_MIPSRESETDELAY	0x00040000
#define DIRECTIVE_DISABLED			0x00080000
#define DIRECTIVE_NOTINMEMORY		0x00100000
#define DIRECTIVE_MANUALSEPARATOR	0x00200000

// file directive flags
#define DIRECTIVE_POS_PHYSICAL		0x00000001
#define DIRECTIVE_POS_VIRTUAL		0x00000002
#define DIRECTIVE_ALIGN_PHYSICAL	0x00000001
#define DIRECTIVE_ALIGN_VIRTUAL		0x00000002
#define DIRECTIVE_ALIGN_FILL		0x00000004

// conditional directive flags
#define DIRECTIVE_COND_IF			0x00000001
#define DIRECTIVE_COND_IFDEF		0x00000002
#define DIRECTIVE_COND_IFNDEF		0x00000003

// data directive flags
#define DIRECTIVE_DATA_8			0x00000001
#define DIRECTIVE_DATA_16			0x00000002
#define DIRECTIVE_DATA_32			0x00000003
#define DIRECTIVE_DATA_64			0x00000004
#define DIRECTIVE_DATA_ASCII		0x00000005
#define DIRECTIVE_DATA_SJIS			0x00000006
#define DIRECTIVE_DATA_CUSTOM		0x00000007
#define DIRECTIVE_DATA_FLOAT		0x00000008
#define DIRECTIVE_DATA_DOUBLE		0x00000009
#define DIRECTIVE_DATA_TERMINATION	0x00000100

// message directive flags
#define DIRECTIVE_MSG_WARNING		0x00000001
#define DIRECTIVE_MSG_ERROR			0x00000002
#define DIRECTIVE_MSG_NOTICE		0x00000003

// MIPS directive flags
#define DIRECTIVE_MIPS_PSX			0x00000001
#define DIRECTIVE_MIPS_PS2			0x00000002
#define DIRECTIVE_MIPS_PSP			0x00000003
#define DIRECTIVE_MIPS_N64			0x00000004
#define DIRECTIVE_MIPS_RSP			0x00000005

// ARM directive flags
#define DIRECTIVE_ARM_GBA			0x00000001
#define DIRECTIVE_ARM_NDS			0x00000002
#define DIRECTIVE_ARM_3DS			0x00000003
#define DIRECTIVE_ARM_BIG			0x00000004
#define DIRECTIVE_ARM_LITTLE		0x00000005

// Z80 directive flags
#define DIRECTIVE_Z80_Z80			0x00000001
#define DIRECTIVE_Z80_GB			0x00000002
#define DIRECTIVE_Z80_GBC			0x00000003

// Area directive flags
#define DIRECTIVE_AREA_SHARED		0x00000001

extern const DirectiveMap directives;
