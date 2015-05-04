#pragma once
#include "Tokenizer.h"

class CAssemblerCommand;
typedef CAssemblerCommand* (*DirectiveFunc)(Tokenizer&,int);

typedef struct {
	const wchar_t* name;
	DirectiveFunc function;
	int flags;
} DirectiveEntry;

#define DIRECTIVE_USERMASK			0x0000FFFF

// Global flags
#define DIRECTIVE_NOCASHON			0x00010000
#define DIRECTIVE_NOCASHOFF			0x00020000
#define DIRECTIVE_MIPSRESETDELAY	0x00040000
#define DIRECTIVE_DISABLED			0x00080000
#define DIRECTIVE_NOTINMEMORY		0x00100000

// file directive flags
#define DIRECTIVE_POS_PHYSICAL		0x00000001
#define DIRECTIVE_POS_VIRTUAL		0x00000002
#define DIRECTIVE_FILE_ALIGN		0x00000001
#define DIRECTIVE_FILE_FILL			0x00000002 

// conditional directive flags
#define DIRECTIVE_COND_IF			0x00000001
#define DIRECTIVE_COND_IFDEF		0x00000002
#define DIRECTIVE_COND_IFNDEF		0x00000003
#define DIRECTIVE_COND_IFARM		0x00000004
#define DIRECTIVE_COND_IFTHUMB		0x00000005

// data directive flags
#define DIRECTIVE_DATA_8			0x00000001
#define DIRECTIVE_DATA_16			0x00000002
#define DIRECTIVE_DATA_32			0x00000003
#define DIRECTIVE_DATA_ASCII		0x00000004
#define DIRECTIVE_DATA_SJIS			0x00000005
#define DIRECTIVE_DATA_CUSTOM		0x00000006
#define DIRECTIVE_DATA_TERMINATION	0x00000100

CAssemblerCommand* parseDirective(Tokenizer& tokenizer, const DirectiveEntry* directiveSet);
CAssemblerCommand* parseGlobalDirective(Tokenizer& tokenizer);