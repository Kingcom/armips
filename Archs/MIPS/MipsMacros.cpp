#include "stdafx.h"
#include "MipsMacros.h"
#include "CMipsInstruction.h"
#include "Core/Common.h"
#include "Mips.h"
#include "MipsOpcodes.h"
#include "CMipsMacro.h"

void MipsMacroLoadOpcode(CMipsInstruction& Opcode, const char* name, const char* argformat, ...)
{
	char str[1024];
	va_list args;
	
	va_start(args,argformat);
	vsprintf(str,argformat,args);
	va_end (args);

	Opcode.Load(name,str);
}

int MipsMacroLi(tMipsMacroValues& Values, int Flags, CMipsInstruction* Opcodes)
{
	int OpcodeCount = 0;

	bool upper = (Flags & MIPSM_UPPER) != 0;
	bool lower = (Flags & MIPSM_LOWER) != 0;

	if ((unsigned)Values.i2 > 0xFFFF)	// may not fit into one opcode
	{
		if ((Values.i2 & 0xFFFF8000) == 0xFFFF8000)
		{
			if (!upper)
			{
				MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"addiu","%s,r0,0x%04X",
					Values.rs.name,Values.i2 & 0xFFFF);
			}
		} else if ((Values.i2 & 0xFFFF) == 0)
		{
			if (lower)
			{
				MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"nop","");
			} else {
				MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"lui","%s,0x%04X",
					Values.rs.name,Values.i2 >> 16);
			}
		} else {	// lui+addiu
			if (Values.i2 & 0x8000) Values.i2 += 0x10000;

			if (upper == false && lower == false)
			{
				upper = lower = true;
			}

			if (upper)
			{
				MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"lui","%s,0x%04X",
					Values.rs.name,Values.i2 >> 16);
			}

			if (lower)
			{
				MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"addiu","%s,%s,0x%04X",
					Values.rs.name,Values.rs.name,Values.i2 & 0xFFFF);
			}
		}
	} else { // definitely fits into one opcode
		if (upper)
		{
			MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"nop","");
		} else {
			MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"ori","%s,r0,0x%04X",Values.rs.name,Values.i2);
		}
	}

	return OpcodeCount;
}

int MipsMacroLoad(tMipsMacroValues& Values, int Flags, CMipsInstruction* Opcodes)
{
	int OpcodeCount = 0;

	if (Values.i2 & 0x8000) Values.i2 += 0x10000;

	if (Flags & MIPSM_UPPER)
	{
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"lui","%s,0x%04X",Values.rs.name,Values.i2 >> 16);
	}

	if (Flags & MIPSM_LOWER)
	{
		if (Flags & MIPSM_B) MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"lb","%s,0x%04X(%s)",
			Values.rs.name,Values.i2 & 0xFFFF,Values.rs.name);
		else if (Flags & MIPSM_BU) MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"lbu","%s,0x%04X(%s)",
			Values.rs.name,Values.i2 & 0xFFFF,Values.rs.name);
		else if (Flags & MIPSM_HW) MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"lh","%s,0x%04X(%s)",
			Values.rs.name,Values.i2 & 0xFFFF,Values.rs.name);
		else if (Flags & MIPSM_HWU) MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"lhu","%s,0x%04X(%s)",
			Values.rs.name,Values.i2 & 0xFFFF,Values.rs.name);
		else if (Flags & MIPSM_W) MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"lw","%s,0x%04X(%s)",
			Values.rs.name,Values.i2 & 0xFFFF,Values.rs.name);
		else Logger::printError(Logger::Error,L"Invalid Load macro");
	}

	return OpcodeCount;
}

int MipsMacroStore(tMipsMacroValues& Values, int Flags, CMipsInstruction* Opcodes)
{
	int OpcodeCount = 0;

	if (Values.i2 & 0x8000) Values.i2 += 0x10000;
	
	if (Flags & MIPSM_UPPER)
	{
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"lui","r1,0x%04X",Values.i2 >> 16);
	}
	
	if (Flags & MIPSM_LOWER)
	{
		if (Flags & MIPSM_B) MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"sb","%s,0x%04X(r1)",
			Values.rs.name,Values.i2 & 0xFFFF);
		else if (Flags & MIPSM_HW) MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"sh","%s,0x%04X(r1)",
			Values.rs.name,Values.i2 & 0xFFFF);
		else if (Flags & MIPSM_W) MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"sw","%s,0x%04X(r1)",
			Values.rs.name,Values.i2 & 0xFFFF);
		else Logger::printError(Logger::Error,L"Invalid Store macro");
	}

	return OpcodeCount;
}

int MipsMacroLoadUnaligned(tMipsMacroValues& Values, int Flags, CMipsInstruction* Opcodes)
{
	int OpcodeCount = 0;
	if (!(Flags & MIPSM_IMM)) Values.i1 = 0;

	if (Flags & MIPSM_HW)
	{
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"lb","r1,0x%04X(%s)",
			Values.i1+1,Values.rs.name);
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"lb","%s,0x%04X(%s)",
			Values.rd.name,Values.i1,Values.rs.name);
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"sll","r1,8");
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"or","%s,r1",
			Values.rd.name );
	} else if (Flags & MIPSM_HWU)
	{
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"lbu","r1,0x%04X(%s)",
			Values.i1+1,Values.rs.name);
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"lbu","%s,0x%04X(%s)",
			Values.rd.name,Values.i1,Values.rs.name);
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"sll","r1,8");
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"or","%s,r1",
			Values.rd.name );
	} else if (Flags & MIPSM_W)
	{
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"lwl","%s,0x%04X(%s)",
			Values.rd.name,Values.i1+3,Values.rs.name);
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"lwr","%s,0x%04X(%s)",
			Values.rd.name,Values.i1,Values.rs.name);
	} else {
		Logger::printError(Logger::Error,L"Invalid Store Unaligned macro");
	}

	return OpcodeCount;
}

int MipsMacroStoreUnaligned(tMipsMacroValues& Values, int Flags, CMipsInstruction* Opcodes)
{
	int OpcodeCount = 0;
	if (!(Flags & MIPSM_IMM)) Values.i1 = 0;

	if (Flags & MIPSM_HW)
	{
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"sb","%s,0x%04X(%s)",
			Values.rd.name,Values.i1,Values.rs.name);
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"srl","r1,%s,8",
			Values.rd.name);
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"sb","r1,0x%04X(%s)",
			Values.i1+1,Values.rs.name);
	} else if (Flags & MIPSM_W)
	{
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"swl","%s,0x%04X(%s)",
			Values.rd.name,Values.i1+3,Values.rs.name);
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"swr","%s,0x%04X(%s)",
			Values.rd.name,Values.i1,Values.rs.name);
	} else {
		Logger::printError(Logger::Error,L"Invalid Store Unaligned macro");
	}

	return OpcodeCount;
}

int MipsMacroBranch(tMipsMacroValues& Values, int Flags, CMipsInstruction* Opcodes)
{
	int OpcodeCount = 0;
	tMipsMacroValues NewValues;
	bool LoadedImmediate = false;

	if (((Flags & MIPSM_IMM) && (unsigned) Values.i1 > 0xFFFF) ||
		(Flags & MIPSM_NE) || (Flags & MIPSM_EQ)) // has to be loaded into r1
	{
		strcpy(NewValues.rs.name,"r1");
		NewValues.i2 = Values.i1;
		OpcodeCount = MipsMacroLi(NewValues,0,Opcodes);
		LoadedImmediate = true;
	}

	if (Flags & MIPSM_NE)
	{
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"bne","%s,r1,0x%08X",
			Values.rs.name,Values.i2);
	} else if (Flags & MIPSM_EQ)
	{
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"beq","%s,r1,0x%08X",
			Values.rs.name,Values.i2);
	} else {
		if (LoadedImmediate == true)
		{
			MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"slt","r1,%s,r1",
				Values.rs.name);
		} else {
			if (Flags & MIPSM_IMM)
			{
				MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"slti","r1,%s,0x%04X",
					Values.rs.name,Values.i1);
			} else {
				MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"slt","r1,%s,%s",
					Values.rs.name,Values.rt.name);
			}
		}

		if (Flags & MIPSM_GE)
		{
			MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"beqz","r1,0x%08X",
				Values.i2);
		} else if (Flags & MIPSM_LT)
		{
			MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"bnez","r1,0x%08X",
				Values.i2);
		} else {
			Logger::printError(Logger::Error,L"Invalid Branch macro");
		}
	}

	return OpcodeCount;
}

int MipsMacroRotate(tMipsMacroValues& Values, int Flags, CMipsInstruction* Opcodes)
{
	int OpcodeCount = 0;

	// PSP has its own rotate opcodes, use those
	if (Mips.GetVersion() == MARCH_PSP)
	{
		if (Flags & MIPSM_IMM)
		{
			if (Flags & MIPSM_LEFT)
			{
				MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"rotr","%s,%s,32-0x%02X",
					Values.rd.name,Values.rs.name,Values.i1);
			} else {
				MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"rotr","%s,%s,0x%02X",
					Values.rd.name,Values.rs.name,Values.i1);
			}
		} else {
			if (Flags & MIPSM_LEFT)
			{
				MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"subu","r1,r0,%s",
					Values.rt.name);
				MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"rotrv","%s,%s,r1",
					Values.rd.name,Values.rs.name);
			} else {
				MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"rotrv","%s,%s,%s",
					Values.rd.name,Values.rs.name,Values.rt.name);
			}
		}

		return OpcodeCount;
	}

	if (Flags & MIPSM_IMM)
	{
		if (Flags & MIPSM_LEFT)
		{
			MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"srl","r1,%s,0x%02X",
				Values.rs.name,32-Values.i1);
			MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"sll","%s,%s,0x%02X",
				Values.rd.name,Values.rs.name,Values.i1);
		} else {
			MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"sll","r1,%s,0x%02X",
				Values.rs.name,32-Values.i1);
			MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"srl","%s,%s,0x%02X",
				Values.rd.name,Values.rs.name,Values.i1);
		}
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"or","%s,r1",
			Values.rd.name);
	} else {
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"subu","r1,r0,%s",
			Values.rt.name);

		if (Flags & MIPSM_LEFT)
		{
			MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"srlv","r1,%s,r1",
				Values.rs.name);
			MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"sllv","%s,%s,%s",
				Values.rd.name,Values.rs.name,Values.rt.name);
		} else {
			MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"sllv","r1,%s,r1",
				Values.rs.name);
			MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"srlv","%s,%s,%s",
				Values.rd.name,Values.rs.name,Values.rt.name);
		}
		MipsMacroLoadOpcode(Opcodes[OpcodeCount++],"or","%s,r1",
			Values.rd.name);
	}

	return OpcodeCount;
}



/* Placeholders
	i = i1 = 16 bit immediate
	I = i2 = 32 bit immediate
	s,t,d = registers */
const tMipsMacro MipsMacros[] = {
	{ "li",		"s,I",		2,	&MipsMacroLi,				MIPSM_IMM },
	{ "li.u",	"s,I",		2,	&MipsMacroLi,				MIPSM_IMM|MIPSM_UPPER },
	{ "li.l",	"s,I",		2,	&MipsMacroLi,				MIPSM_IMM|MIPSM_LOWER },
	{ "la",		"s,I",		2,	&MipsMacroLi,				MIPSM_IMM },
	{ "la.u",	"s,I",		2,	&MipsMacroLi,				MIPSM_IMM|MIPSM_UPPER },
	{ "la.l",	"s,I",		2,	&MipsMacroLi,				MIPSM_IMM|MIPSM_LOWER },

	{ "lb",		"s,I",		2,	&MipsMacroLoad,				MIPSM_B|MIPSM_UPPER|MIPSM_LOWER },
	{ "lbu",	"s,I",		2,	&MipsMacroLoad,				MIPSM_BU|MIPSM_UPPER|MIPSM_LOWER },
	{ "lh",		"s,I",		2,	&MipsMacroLoad,				MIPSM_HW|MIPSM_UPPER|MIPSM_LOWER },
	{ "lhu",	"s,I",		2,	&MipsMacroLoad,				MIPSM_HWU|MIPSM_UPPER|MIPSM_LOWER },
	{ "lw",		"s,I",		2,	&MipsMacroLoad,				MIPSM_W|MIPSM_UPPER|MIPSM_LOWER },

	{ "lb.u",	"s,I",		2,	&MipsMacroLoad,				MIPSM_B|MIPSM_UPPER },
	{ "lbu.u",	"s,I",		2,	&MipsMacroLoad,				MIPSM_BU|MIPSM_UPPER },
	{ "lh.u",	"s,I",		2,	&MipsMacroLoad,				MIPSM_HW|MIPSM_UPPER },
	{ "lhu.u",	"s,I",		2,	&MipsMacroLoad,				MIPSM_HWU|MIPSM_UPPER },
	{ "lw.u",	"s,I",		2,	&MipsMacroLoad,				MIPSM_W|MIPSM_UPPER },
	
	{ "lb.l",	"s,I",		2,	&MipsMacroLoad,				MIPSM_B|MIPSM_LOWER },
	{ "lbu.l",	"s,I",		2,	&MipsMacroLoad,				MIPSM_BU|MIPSM_LOWER },
	{ "lh.l",	"s,I",		2,	&MipsMacroLoad,				MIPSM_HW|MIPSM_LOWER },
	{ "lhu.l",	"s,I",		2,	&MipsMacroLoad,				MIPSM_HWU|MIPSM_LOWER },
	{ "lw.l",	"s,I",		2,	&MipsMacroLoad,				MIPSM_W|MIPSM_LOWER },

	{ "ulh",	"d,i(s)",	4,	&MipsMacroLoadUnaligned,	MIPSM_HW|MIPSM_IMM },
	{ "ulh",	"d,(s)",	4,	&MipsMacroLoadUnaligned,	MIPSM_HW },
	{ "ulhu",	"d,i(s)",	4,	&MipsMacroLoadUnaligned,	MIPSM_HWU|MIPSM_IMM },
	{ "ulhu",	"d,(s)",	4,	&MipsMacroLoadUnaligned,	MIPSM_HWU },
	{ "ulw",	"d,i(s)",	4,	&MipsMacroLoadUnaligned,	MIPSM_W|MIPSM_IMM },
	{ "ulw",	"d,(s)",	4,	&MipsMacroLoadUnaligned,	MIPSM_W },

	{ "sb",		"s,I",		2,	&MipsMacroStore,			MIPSM_B|MIPSM_UPPER|MIPSM_LOWER },
	{ "sh",		"s,I",		2,	&MipsMacroStore,			MIPSM_HW|MIPSM_UPPER|MIPSM_LOWER },
	{ "sw",		"s,I",		2,	&MipsMacroStore,			MIPSM_W|MIPSM_UPPER|MIPSM_LOWER },
	
	{ "sb.u",	"s,I",		2,	&MipsMacroStore,			MIPSM_B|MIPSM_UPPER },
	{ "sh.u",	"s,I",		2,	&MipsMacroStore,			MIPSM_HW|MIPSM_UPPER },
	{ "sw.u",	"s,I",		2,	&MipsMacroStore,			MIPSM_W|MIPSM_UPPER },

	{ "sb.l",	"s,I",		2,	&MipsMacroStore,			MIPSM_B|MIPSM_LOWER },
	{ "sh.l",	"s,I",		2,	&MipsMacroStore,			MIPSM_HW|MIPSM_LOWER },
	{ "sw.l",	"s,I",		2,	&MipsMacroStore,			MIPSM_W|MIPSM_LOWER },

	{ "ush",	"d,i(s)",	3,	&MipsMacroStoreUnaligned,	MIPSM_HW|MIPSM_IMM },
	{ "ush",	"d,(s)",	3,	&MipsMacroStoreUnaligned,	MIPSM_HW },
	{ "usw",	"d,i(s)",	3,	&MipsMacroStoreUnaligned,	MIPSM_W|MIPSM_IMM },
	{ "usw",	"d,(s)",	3,	&MipsMacroStoreUnaligned,	MIPSM_W },
	
	{ "blt",	"s,t,I",	4,	&MipsMacroBranch,			MIPSM_LT|MIPSM_DONTWARNDELAYSLOT },
	{ "blt",	"s,i,I",	4,	&MipsMacroBranch,			MIPSM_LT|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ "bge",	"s,t,I",	4,	&MipsMacroBranch,			MIPSM_GE|MIPSM_DONTWARNDELAYSLOT },
	{ "bge",	"s,i,I",	4,	&MipsMacroBranch,			MIPSM_GE|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ "bne",	"s,i,I",	4,	&MipsMacroBranch,			MIPSM_NE|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ "beq",	"s,i,I",	4,	&MipsMacroBranch,			MIPSM_EQ|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },

	{ "rol",	"d,s,t",	4,	&MipsMacroRotate,			MIPSM_LEFT },
	{ "rol",	"d,s,i",	4,	&MipsMacroRotate,			MIPSM_LEFT|MIPSM_IMM },
	{ "ror",	"d,s,t",	4,	&MipsMacroRotate,			MIPSM_RIGHT },
	{ "ror",	"d,s,i",	4,	&MipsMacroRotate,			MIPSM_RIGHT|MIPSM_IMM },

	{ NULL, NULL, 0, NULL, 0 }
};

bool MipsCheckMacroParsing(const char* Opcode, const char* Arguments, tMipsMacroVars& Vars)
{
	int RetLen;

	while (*Arguments == ' ' || *Arguments == '\t') Arguments++;
	if (*Opcode == 0 && *Arguments == 0) return true;
	while (*Opcode != 0)
	{
		while (*Arguments == ' ' || *Arguments == '\t') Arguments++;
		if (*Arguments == 0) return false;
		switch (*Opcode)
		{
		case 's':
			if (MipsGetRegister(Arguments,RetLen,Vars.rs) == false) return false;
			Arguments += RetLen;
			Opcode++;
			break;
		case 'd':
			if (MipsGetRegister(Arguments,RetLen,Vars.rd) == false) return false;
			Arguments += RetLen;
			Opcode++;
			break;
		case 't':
			if (MipsGetRegister(Arguments,RetLen,Vars.rt) == false) return false;
			Arguments += RetLen;
			Opcode++;
			break;
		case 'i':
			if (MipsCheckImmediate(Arguments,Vars.i1,RetLen) == false) return false;
			Arguments += RetLen;
			Opcode++;
			break;
		case 'I':
			if (MipsCheckImmediate(Arguments,Vars.i2,RetLen) == false) return false;
			Arguments += RetLen;
			Opcode++;
			break;
		default:
			if (*Opcode++ != *Arguments++) return false;
			break;
		}
	}
	
	while (*Arguments == ' ' || *Arguments == '\t') Arguments++;
	if (*Arguments != 0)	return false;	// there's something else, bad

	if (Vars.i1.isLoaded())
	{
		if (Vars.i1.check() == false)
		{
			Vars.NoCheckError = true;
			return false;
		}
	}

	if (Vars.i2.isLoaded())
	{
		if (Vars.i2.check() == false)
		{
			Vars.NoCheckError = true;
			return false;
		}
	}

	return true;
}

bool MipsCheckMacro(char* Opcode, char* Arguments)
{
	tMipsMacroVars Vars;
	Vars.NoCheckError = false;

	bool ParamFail = false;
	for (int y = 0; MipsMacros[y].name != NULL; y++)
	{
		if (strcmp(Opcode,MipsMacros[y].name) == 0)
		{
			if (MipsCheckMacroParsing(MipsMacros[y].args,Arguments,Vars) == true)
			{
				CMipsMacro* Macro = new CMipsMacro(y,Vars);
				AddAssemblerCommand(Macro);
				return true;
			} else {
				ParamFail = true;
			}
		}
	}

	if (ParamFail == true)
	{
		return false;
	}
	return false;
}
