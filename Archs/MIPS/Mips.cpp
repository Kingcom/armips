#include "stdafx.h"
#include "Mips.h"
#include "CMipsInstruction.h"
#include "Core/Common.h"
#include "MipsMacros.h"
#include "Core/Directives.h"

CMipsArchitecture Mips;

typedef struct {
	int number;
	int instructions;
} tRegisterDelay;

class CMipsDelayManager
{
public:
	void Advance();
	bool CheckDelay(tMipsOpcodeVariables& Vars, int flags);
private:
	bool CheckGprDelay(tMipsOpcodeVariables& Vars, int flags);
	bool CheckFprDelay(tMipsOpcodeVariables& Vars, int flags);
	tRegisterDelay Gpr;
	tRegisterDelay Fpr;
};

void CMipsDelayManager::Advance()
{
	if (Gpr.instructions != 0) Gpr.instructions--;
	if (Fpr.instructions != 0) Fpr.instructions--;
}

bool CMipsDelayManager::CheckGprDelay(tMipsOpcodeVariables& Vars, int flags)
{
	tMipsRegisterInfo* Reg = NULL;
	if ((flags & O_RD) && Vars.rd.Number == Gpr.number) Reg = &Vars.rd;
	else if ((flags & O_RS) && Vars.rs.Number == Gpr.number) Reg = &Vars.rs;
	else if ((flags & O_RT) && Vars.rt.Number == Gpr.number) Reg = &Vars.rt;
	else if ((flags & O_RDT) && Vars.rd.Number == Gpr.number) Reg = &Vars.rd;
	else if ((flags & O_RSD) && Vars.rs.Number == Gpr.number) Reg = &Vars.rs;
	else if ((flags & O_RST) && Vars.rs.Number == Gpr.number) Reg = &Vars.rs;

	if (Reg == NULL) return false;
	QueueError(ERROR_WARNING,"register %s may not be available due to load delay",Reg->Name);
	return true;
}

bool CMipsDelayManager::CheckFprDelay(tMipsOpcodeVariables& Vars, int flags)
{
	tMipsRegisterInfo* Reg = NULL;
	if ((flags & MO_FRD) && Vars.rd.Number == Fpr.number) Reg = &Vars.rd;
	else if ((flags & MO_FRS) && Vars.rs.Number == Fpr.number) Reg = &Vars.rs;
	else if ((flags & MO_FRT) && Vars.rt.Number == Fpr.number) Reg = &Vars.rt;
	else if ((flags & MO_FRDT) && Vars.rd.Number == Fpr.number) Reg = &Vars.rd;
	else if ((flags & MO_FRSD) && Vars.rs.Number == Fpr.number) Reg = &Vars.rs;
	else if ((flags & MO_FRST) && Vars.rs.Number == Fpr.number) Reg = &Vars.rs;

	if (Reg == NULL) return false;
	QueueError(ERROR_WARNING,"register %s may not be available due to load delay",Reg->Name);
	return true;
}

bool CMipsDelayManager::CheckDelay(tMipsOpcodeVariables& Vars, int flags)
{
	if (Gpr.instructions != 0) if (CheckGprDelay(Vars,flags) == true) return true;
	if (Fpr.instructions != 0) if (CheckFprDelay(Vars,flags) == true) return true;
	return false;
}

const tMipsRegister MipsRegister[] = {
	{ "r0", 0, 2 }, { "zero", 0, 4}, { "$0", 0, 2 },
	{ "at", 1, 2 }, { "r1", 1, 2 }, { "$1", 1, 2 },
	{ "v0", 2, 2 }, { "r2", 2, 2 }, { "$v0", 2, 3 },
	{ "v1", 3, 2 }, { "r3", 3, 2 }, { "$v1", 3, 3 },
	{ "a0", 4, 2 }, { "r4", 4, 2 }, { "$a0", 4, 3 },
	{ "a1", 5, 2 }, { "r5", 5, 2 }, { "$a1", 5, 3 },
	{ "a2", 6, 2 }, { "r6", 6, 2 }, { "$a2", 6, 3 },
	{ "a3", 7, 2 }, { "r7", 7, 2 }, { "$a3", 7, 3 },
	{ "t0", 8, 2 }, { "r8", 8, 2 }, { "$t0", 8, 3 },
	{ "t1", 9, 2 }, { "r9", 9, 2 }, { "$t1", 9, 3 },
	{ "t2", 10, 2 }, { "r10", 10, 3 }, { "$t2", 10, 3 },
	{ "t3", 11, 2 }, { "r11", 11, 3 }, { "$t3", 11, 3 },
	{ "t4", 12, 2 }, { "r12", 12, 3 }, { "$t4", 12, 3 },
	{ "t5", 13, 2 }, { "r13", 13, 3 }, { "$t5", 13, 3 },
	{ "t6", 14, 2 }, { "r14", 14, 3 }, { "$t6", 14, 3 },
	{ "t7", 15, 2 }, { "r15", 15, 3 }, { "$t7", 15, 3 },
	{ "s0", 16, 2 }, { "r16", 16, 3 }, { "$s0", 16, 3 },
	{ "s1", 17, 2 }, { "r17", 17, 3 }, { "$s1", 17, 3 },
	{ "s2", 18, 2 }, { "r18", 18, 3 }, { "$s2", 18, 3 },
	{ "s3", 19, 2 }, { "r19", 19, 3 }, { "$s3", 19, 3 },
	{ "s4", 20, 2 }, { "r20", 20, 3 }, { "$s4", 20, 3 },
	{ "s5", 21, 2 }, { "r21", 21, 3 }, { "$s5", 21, 3 },
	{ "s6", 22, 2 }, { "r22", 22, 3 }, { "$s6", 22, 3 },
	{ "s7", 23, 2 }, { "r23", 23, 3 }, { "$s7", 23, 3 },
	{ "t8", 24, 2 }, { "r24", 24, 3 }, { "$t8", 24, 3 },
	{ "t9", 25, 2 }, { "r25", 25, 3 }, { "$t9", 25, 3 },
	{ "k0", 26, 2 }, { "r26", 26, 3 }, { "$k0", 26, 3 },
	{ "k1", 27, 2 }, { "r27", 27, 3 }, { "$k1", 27, 3 },
	{ "gp", 28, 2 }, { "r28", 28, 3 }, { "$gp", 28, 3 },
	{ "sp", 29, 2 }, { "r29", 29, 3 }, { "$sp", 29, 3 },
	{ "fp", 30, 2 }, { "r30", 30, 3 }, { "$fp", 30, 3 },
	{ "ra", 31, 2 }, { "r31", 31, 3 }, { "$ra", 31, 3 },
	{ NULL, -1, 0}
};


const tMipsRegister MipsFloatRegister[] = {
	{ "f0", 0, 2},		{ "$f0", 0, 3 },
	{ "f1", 1, 2},		{ "$f1", 1, 3 },
	{ "f2", 2, 2},		{ "$f2", 2, 3 },
	{ "f3", 3, 2},		{ "$f3", 3, 3 },
	{ "f4", 4, 2},		{ "$f4", 4, 3 },
	{ "f5", 5, 2},		{ "$f5", 5, 3 },
	{ "f6", 6, 2},		{ "$f6", 6, 3 },
	{ "f7", 7, 2},		{ "$f7", 7, 3 },
	{ "f8", 8, 2},		{ "$f8", 8, 3 },
	{ "f9", 9, 2},		{ "$f9", 9, 3 },
	{ "f10", 10, 3},	{ "$f10", 10, 4 },
	{ "f11", 11, 3},	{ "$f11", 11, 4 },
	{ "f12", 12, 3},	{ "$f12", 12, 4 },
	{ "f13", 13, 3},	{ "$f13", 13, 4 },
	{ "f14", 14, 3},	{ "$f14", 14, 4 },
	{ "f15", 15, 3},	{ "$f15", 15, 4 },
	{ "f16", 16, 3},	{ "$f16", 16, 4 },
	{ "f17", 17, 3},	{ "$f17", 17, 4 },
	{ "f18", 18, 3},	{ "$f18", 18, 4 },
	{ "f19", 19, 3},	{ "$f19", 19, 4 },
	{ "f20", 20, 3},	{ "$f20", 20, 4 },
	{ "f21", 21, 3},	{ "$f21", 21, 4 },
	{ "f22", 22, 3},	{ "$f22", 22, 4 },
	{ "f23", 23, 3},	{ "$f23", 23, 4 },
	{ "f24", 24, 3},	{ "$f24", 24, 4 },
	{ "f25", 25, 3},	{ "$f25", 25, 4 },
	{ "f26", 26, 3},	{ "$f26", 26, 4 },
	{ "f27", 27, 3},	{ "$f27", 27, 4 },
	{ "f28", 28, 3},	{ "$f28", 28, 4 },
	{ "f29", 29, 3},	{ "$f29", 29, 4 },
	{ "f30", 30, 3},	{ "$f30", 30, 4 },
	{ "f31", 31, 3},	{ "$f31", 31, 4 }
};

bool MipsDirectiveResetDelay(ArgumentList& List, int flags)
{
	Mips.SetIgnoreDelay(true);
	return true;
}

bool MipsDirectiveFixLoadDelay(ArgumentList& List, int flags)
{
	Mips.SetFixLoadDelay(true);
	return true;
}

const tDirective MipsDirectives[] = {
	{ ".resetdelay",	0,	0,	&MipsDirectiveResetDelay,	0 },
	{ ".fixloaddelay",	0,	0,	&MipsDirectiveFixLoadDelay,	0 },
	{ NULL,				0,	0,	NULL,	0 }
};

bool CMipsArchitecture::AssembleDirective(const std::wstring& name, const std::wstring& args)
{
	if (directiveAssembleGlobal(name,args) == true) return true;
	return directiveAssemble(MipsDirectives,name,args);
}

CMipsArchitecture::CMipsArchitecture()
{
	FixLoadDelay = false;
	IgnoreLoadDelay = false;
	LoadDelay = false;
	LoadDelayRegister = 0;
	DelaySlot = false;
	Version = 0;
}

void CMipsArchitecture::AssembleOpcode(char* name, char* args)
{
/*	// erst directives prüfen
	for (int z = 0; MipsDirectives[z].name != NULL; z++)
	{
		if (strcmp(name,MipsDirectives[z].name) == 0)
		{
			ExecuteDirective(MipsDirectives[z],args);
			return;
		}
	}*/

	// dann macros/opcodes
	if (MipsCheckMacro(name,args) == false)
	{
		CMipsInstruction* Opcode = new CMipsInstruction();
		if (Opcode->Load(name,args) == false)
		{
			delete Opcode;
			return;
		}
		AddAssemblerCommand(Opcode);
		Global.RamPos += 4;
	}
	SetIgnoreDelay(false);
}

void CMipsArchitecture::NextSection()
{
	LoadDelay = false;
	LoadDelayRegister = 0;
	DelaySlot = false;
}

int CMipsArchitecture::GetWordSize()
{
	// keine ahnung ob das alles stimmt...
	if (Version & MARCH_PSX) return 4;
	if (Version & MARCH_N64) return 4;
	if (Version & MARCH_PS2) return 8;
	if (Version & MARCH_PSP) return 8;
	return 0;
}

void CMipsArchitecture::SetLoadDelay(bool Delay, int Register)
{
	LoadDelay = Delay;
	LoadDelayRegister = Register;
}

bool MipsGetRegister(char* source, int& RetLen, tMipsRegisterInfo& Result)
{
	for (int z = 0; MipsRegister[z].name != NULL; z++)
	{
		int len = MipsRegister[z].len;
		if (strncmp(MipsRegister[z].name,source,len) == 0)	// erstmal in ordnung
		{
			if (source[len] == ',' || source[len] == '\n'  || source[len] == 0
				|| source[len] == ')'  || source[len] == '(' || source[len] == '-')	// eins hiervon MUSS nach nem register kommen
			{
				memcpy(Result.Name,source,len);
				Result.Name[len] = 0;
				Result.Number = MipsRegister[z].num;
				RetLen = len;
				return true;
			}
		}
	}
	return false;
}

int MipsGetRegister(char* source, int& RetLen)
{
	for (int z = 0; MipsRegister[z].name != NULL; z++)
	{
		int len = MipsRegister[z].len;
		if (strncmp(MipsRegister[z].name,source,len) == 0)	// erstmal in ordnung
		{
			if (source[len] == ',' || source[len] == '\n'  || source[len] == 0
				|| source[len] == ')'  || source[len] == '(' || source[len] == '-')	// eins hiervon MUSS nach nem register kommen
			{
				RetLen = len;
				return MipsRegister[z].num;
			}
		}
	}
	return -1;
}


bool MipsGetFloatRegister(char* source, int& RetLen, tMipsRegisterInfo& Result)
{
	for (int z = 0; MipsFloatRegister[z].name != NULL; z++)
	{
		int len = MipsFloatRegister[z].len;
		if (strncmp(MipsFloatRegister[z].name,source,len) == 0)	// erstmal in ordnung
		{
			if (source[len] == ',' || source[len] == '\n'  || source[len] == 0
				|| source[len] == ')'  || source[len] == '(' || source[len] == '-')	// eins hiervon MUSS nach nem register kommen
			{
				memcpy(Result.Name,source,len);
				Result.Name[len] = 0;
				Result.Number = MipsFloatRegister[z].num;
				RetLen = len;
				return true;
			}
		}
	}
	return false;
}

int MipsGetFloatRegister(char* source, int& RetLen)
{
	for (int z = 0; MipsFloatRegister[z].name != NULL; z++)
	{
		int len = MipsFloatRegister[z].len;
		if (strncmp(MipsFloatRegister[z].name,source,len) == 0)	// erstmal in ordnung
		{
			if (source[len] == ',' || source[len] == '\n'  || source[len] == 0
				|| source[len] == ')'  || source[len] == '(' || source[len] == '-')	// eins hiervon MUSS nach nem register kommen
			{
				RetLen = len;
				return MipsFloatRegister[z].num;
			}
		}
	}
	return -1;
}



bool MipsCheckImmediate(char* Source, char* Dest, int& RetLen, CStringList& List)
{
	int BufferPos = 0;
	int l;

	if (MipsGetRegister(Source,l) != -1)	// fehler ende
	{
		return false;
	}

	int SourceLen = 0;

	while (true)
	{
		if (*Source == '\'' && *(Source+2) == '\'')
		{
			Dest[BufferPos++] = *Source++;
			Dest[BufferPos++] = *Source++;
			Dest[BufferPos++] = *Source++;
			SourceLen+=3;
			continue;
		}

		if (*Source == 0 || *Source == '\n' || *Source == ',')
		{
			Dest[BufferPos] = 0;
			break;
		}
		if ( *Source == ' ' || *Source == '\t')
		{
			Source++;
			SourceLen++;
			continue;
		}


		if (*Source == '(')	// könnte auch durch ne klammer kommen
		{
			if (MipsGetRegister(Source+1,l) != -1)	// ende
			{
				Dest[BufferPos] = 0;
				break;
			}
		}
		Dest[BufferPos++] = *Source++;
		SourceLen++;
	}

	if (BufferPos == 0) return false;
	if (ConvertInfixToPostfix(Dest,List) == false) return false;
//	if (CheckPostfix(List) == false) return false;	// später machen für genauere fehlermeldungen?

	RetLen = SourceLen;
	return true;
}