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

CAssemblerCommand* parseDirective(Tokenizer& tokenizer, const DirectiveEntry* directiveSet);
CAssemblerCommand* parseGlobalDirective(Tokenizer& tokenizer);