#pragma once
#include "Util/CommonClasses.h"


typedef bool (*directivefunc)(ArgumentList&,int);

#define DIRECTIVE_USERMASK			0x0000FFFF

// Global flags
#define DIRECTIVE_NOCASHON			0x00010000
#define DIRECTIVE_NOCASHOFF			0x00020000
#define DIRECTIVE_MIPSRESETDELAY	0x00040000
#define DIRECTIVE_DISABLED			0x00080000
#define DIRECTIVE_NOTINMEMORY		0x00100000

// Directive specific flags
#define DIRECTIVE_COND_IF			0x00000001
#define DIRECTIVE_COND_ELSE			0x00000002
#define DIRECTIVE_COND_ELSEIF		0x00000003
#define DIRECTIVE_COND_ENDIF		0x00000004
#define DIRECTIVE_COND_IFDEF		0x00000005
#define DIRECTIVE_COND_IFNDEF		0x00000006
#define DIRECTIVE_COND_ELSEIFDEF	0x00000007
#define DIRECTIVE_COND_ELSEIFNDEF	0x00000008
#define DIRECTIVE_COND_IFARM		0x00000009
#define DIRECTIVE_COND_IFTHUMB		0x0000000A

#define DIRECTIVE_AREA_START		0x00000001
#define DIRECTIVE_AREA_END			0x00000002

#define DIRECTIVE_MSG_WARNING		0x00000001
#define DIRECTIVE_MSG_ERROR			0x00000002
#define DIRECTIVE_MSG_NOTICE		0x00000003

#define DIRECTIVE_DATA_8			0x00000001
#define DIRECTIVE_DATA_16			0x00000002
#define DIRECTIVE_DATA_32			0x00000004
#define DIRECTIVE_DATA_ASCII		0x00000010

#define DIRECTIVE_STR_NOTERMINATION	0x00000001

typedef struct {
	const wchar_t* name;
	short minparam;
	short maxparam;
	directivefunc Function;
	int Flags;
} tDirective;

extern const tDirective Directives[];
bool executeDirective(const tDirective& Directive, const std::wstring& args);
bool splitArguments(ArgumentList& list, const std::wstring& args);
bool directiveAssemble(const tDirective* directiveSet, const std::wstring& name, const std::wstring& arguments);
bool directiveAssembleGlobal(const std::wstring& name, const std::wstring& arguments);
