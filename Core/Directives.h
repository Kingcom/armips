#pragma once
#include "Util/CommonClasses.h"


typedef bool (*directivefunc)(CArgumentList&,int);

#define DIRECTIVE_NOCASHON			0x00000001
#define DIRECTIVE_NOCASHOFF			0x00000002
#define DIRECTIVE_MIPSRESETDELAY	0x00000004
#define DIRECTIVE_DISABLED			0x00800000

#define DIRECTIVE_OPEN				0x01000000
#define DIRECTIVE_CREATE			0x02000000
#define DIRECTIVE_COPY				0x03000000
#define DIRECTIVE_CLOSE				0x04000000
#define DIRECTIVE_ORG				0x05000000
#define DIRECTIVE_ORGA				0x06000000
#define DIRECTIVE_INCBIN			0x07000000
#define DIRECTIVE_ALIGN				0x08000000
#define DIRECTIVE_FILL				0x09000000
#define DIRECTIVE_HEADERSIZE		0x0A000000

#define DIRECTIVE_IF				0x01000000
#define DIRECTIVE_ELSE				0x02000000
#define DIRECTIVE_ELSEIF			0x03000000
#define DIRECTIVE_ENDIF				0x04000000
#define DIRECTIVE_IFDEF				0x05000000
#define DIRECTIVE_IFNDEF			0x06000000
#define DIRECTIVE_ELSEIFDEF			0x07000000
#define DIRECTIVE_ELSEIFNDEF		0x08000000
#define DIRECTIVE_IFARM				0x10000000
#define DIRECTIVE_IFTHUMB			0x20000000


#define DIRECTIVE_AREASTART			0x01000000
#define DIRECTIVE_AREAEND			0x02000000

#define DIRECTIVE_WARNING			0x01000000
#define DIRECTIVE_ERROR				0x02000000
#define DIRECTIVE_NOTICE			0x03000000

#define DIRECTIVE_PARAMFIELD		0xFF000000

#define DIRECTIVE_DATA8				0x00000400
#define DIRECTIVE_DATA16			0x00000800
#define DIRECTIVE_DATA32			0x00001000
#define DIRECTIVE_ASCII				0x00002000


typedef struct {
	char* name;
	short minparam;
	short maxparam;
	directivefunc Function;
	int Flags;
} tDirective;

extern const tDirective Directives[];
bool ExecuteDirective(const tDirective& Directive, char* args);
bool SplitArguments(CArgumentList& List, char* args);
bool DirectiveAssemble(const tDirective* DirectiveSet, char* Name, char* Arguments);
bool DirectiveAssembleGlobal(char* Name, char* Arguments);
