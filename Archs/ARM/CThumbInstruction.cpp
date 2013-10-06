#include "stdafx.h"
#include "CThumbInstruction.h"
#include "Core/Common.h"
#include "Arm.h"
#include "ThumbOpcodes.h"

CThumbInstruction::CThumbInstruction()
{
	NoCheckError = false;
	Loaded = false;
	OpcodeSize = 0;
}

bool CThumbInstruction::Load(char *Name, char *Params)
{
	bool paramfail = false;
	NoCheckError = false;

	for (int z = 0; ThumbOpcodes[z].name != NULL; z++)
	{
		if (strcmp(Name,ThumbOpcodes[z].name) == 0)
		{
			if (LoadEncoding(ThumbOpcodes[z],Params) == true)
			{
				Loaded = true;
				return true;
			}
			paramfail = true;
		}
	}

	if (NoCheckError == false)
	{
		if (paramfail == true)
		{
			PrintError(ERROR_ERROR,"THUMB parameter failure \"%s\"",Params);
		} else {
			PrintError(ERROR_ERROR,"Invalid THUMB opcode \"%s\"",Name);
		}
	}
	return false;
}

typedef struct {
	char Character;
	char MaxNum;
	short StructOffset;
} tThumbRegisterLookup;

#include <stddef.h>

const tThumbRegisterLookup RegisterLookup[] = {
	{ 'd',	7,	offsetof(tThumbOpcodeVariables,rd) },
	{ 's',	7,	offsetof(tThumbOpcodeVariables,rs) },
	{ 'n',	7,	offsetof(tThumbOpcodeVariables,rn) },
	{ 'o',	7,	offsetof(tThumbOpcodeVariables,ro) },
	{ 'D',	15,	offsetof(tThumbOpcodeVariables,rd) },
	{ 'S',	15,	offsetof(tThumbOpcodeVariables,rs) },
	{ 0,	0,				-1 }
};

bool CThumbInstruction::LoadEncoding(const tThumbOpcode& SourceOpcode, char* Line)
{
	char ImmediateBuffer[512];

	int p,RetLen;
	CStringList List;
	bool Immediate = false;

	char* SourceEncoding = SourceOpcode.mask;
	char* OriginalLine = Line;

	while (*Line == ' ' || *Line == '\t') Line++;
	tArmRegisterInfo* Info;

	if (!(*SourceEncoding == 0 && *Line == 0))
	{
		while (*SourceEncoding != NULL)
		{
			while (*Line == ' ' || *Line == '\t') Line++;
//			if (*Line == 0) return false;

			switch (*SourceEncoding)
			{
			case 'd': case 's': case 'n': case 'o':
			case 'D': case 'S':	// registers
				for (int i = 0; ; i++)
				{
					if (RegisterLookup[i].Character == *SourceEncoding)
					{
						Info = (tArmRegisterInfo*)((int) (&Vars) + RegisterLookup[i].StructOffset);
						if (ArmGetRegister(Line,RetLen,*Info) == false) return false;
						if (Info->Number > RegisterLookup[i].MaxNum) return false;
						Line += RetLen;
						SourceEncoding++;
						break;
					}
				}
				break;
/*			case 'd':	// low reg
				if (ArmGetRegister(Line,RetLen,Vars.rd) == false) return false;
				if (Vars.rd.Number > 7) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 's':	// low reg
				if (ArmGetRegister(Line,RetLen,Vars.rs) == false) return false;
				if (Vars.rs.Number > 7) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'n':	// low reg
				if (ArmGetRegister(Line,RetLen,Vars.rn) == false) return false;
				if (Vars.rn.Number > 7) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'o':	// low reg
				if (ArmGetRegister(Line,RetLen,Vars.ro) == false) return false;
				if (Vars.ro.Number > 7) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'D':	// high reg
				if (ArmGetRegister(Line,RetLen,Vars.rd) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'S':	// high reg
				if (ArmGetRegister(Line,RetLen,Vars.rs) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;*/
			case 'I':	// immediate
			case 'i':
				if (ArmCheckImmediate(Line,ImmediateBuffer,RetLen,List) == false) return false;
				Vars.ImmediateBitLen = *(SourceEncoding+1);
				Line += RetLen;
				SourceEncoding += 2;
				break;
			case 'r':	// forced register
				if (ArmGetRegister(Line,RetLen) != *(SourceEncoding+1)) return false;
				Line += RetLen;
				SourceEncoding += 2;
				break;
			case 'R':	// rlist
				memcpy(&p,&SourceEncoding[1],2);
				if (ArmGetRlist(Line,RetLen,p,Vars.rlist) == false) return false;
				memcpy(Vars.RlistStr,Line,RetLen);
				Vars.RlistStr[RetLen] = 0;
				Line += RetLen;
				SourceEncoding += 3;
				break;
			case '/':	// optionale zeichen
				if (*Line == *(SourceEncoding+1)) Line++;
				SourceEncoding += 2;
				break;
			default:	// alles andere
				if (*SourceEncoding++ != *Line++) return false;
				break;
			}
		}
	}

	while (*Line == ' ' || *Line == '\t') Line++;
	if (*Line != 0)	return false;	// da ist noch mehr, nicht gut

	// opcode ist ok - jetzt noch alle weiteren flags setzen
	Opcode = SourceOpcode;

	if (Opcode.flags & THUMB_IMMEDIATE)
	{
		if (CheckPostfix(List,true) == false)
		{
			PrintError(ERROR_ERROR,"Invalid expression \"%s\"",ImmediateBuffer);
			NoCheckError = true;
			return false;
		}
		Vars.ImmediateExpression.Load(List);
	}

	OpcodeSize = Opcode.flags & THUMB_LONG ? 4 : 2;
	return true;
}

bool CThumbInstruction::Validate()
{
	CStringList List;
	RamPos = Global.RamPos;


	if (RamPos & 1)
	{
		QueueError(ERROR_WARNING,"Opcode not halfword aligned");
	}

	if (Opcode.flags & THUMB_DS)
	{
		Vars.rs = Vars.rd;
	}

	if (Opcode.flags & THUMB_IMMEDIATE)
	{
		if (ParsePostfix(Vars.ImmediateExpression,&List,Vars.Immediate) == false)
		{
			if (List.GetCount() == 0)
			{
				QueueError(ERROR_ERROR,"Invalid expression");
			} else {
				for (int l = 0; l < List.GetCount(); l++)
				{
					QueueError(ERROR_ERROR,List.GetEntry(l));
				}
			}
			return false;
		}
		Vars.OriginalImmediate = Vars.Immediate;

		if (Opcode.flags & THUMB_BRANCH)
		{
			if (Opcode.flags & THUMB_EXCHANGE)
			{
				if (Vars.Immediate & 3)
				{
					QueueError(ERROR_ERROR,"Branch target must be word aligned");
					return false;
				}
			} else {
				if (Vars.Immediate & 1)
				{
					QueueError(ERROR_ERROR,"Branch target must be halfword aligned");
					return false;
				}
			}

			int num = (Vars.Immediate-RamPos-4);
			
			if (num >= (1 << Vars.ImmediateBitLen) || num < (0-(1 << Vars.ImmediateBitLen)))
			{
				QueueError(ERROR_ERROR,"Branch target %08X out of range",Vars.Immediate);
				return false;
			}

			Vars.Immediate = num >> 1;
			if (Opcode.flags & THUMB_EXCHANGE)
			{
				Vars.Immediate += Vars.Immediate&1;
			}
		} else if (Opcode.flags & THUMB_WORD)
		{
			if (Vars.Immediate & 3)	// darf nicht sein
			{
				QueueError(ERROR_ERROR,"Immediate value must be a multiple of 4");
				return false;
			}
			Vars.Immediate >>= 2;
		} else if (Opcode.flags & THUMB_HALFWORD)
		{
			if (Vars.Immediate & 1)	// darf nicht sein
			{
				QueueError(ERROR_ERROR,"Immediate value must be a multiple of 2");
				return false;
			}
			Vars.Immediate >>= 1;
		} else if (Opcode.flags & THUMB_POOL)
		{
			int pos;
			if ((pos = Arm.AddToCurrentPool(Vars.Immediate)) == -1)
			{
				QueueError(ERROR_ERROR,"Unable to find literal pool");
				return false;
			}
			pos = pos-((Global.RamPos+4) & 0xFFFFFFFD);
			if (pos < 0 || pos > 1020)
			{
				QueueError(ERROR_ERROR,"Literal pool out of range");
				return false;
			}
			Vars.Immediate = pos >> 2;
		} else if (Opcode.flags & THUMB_PCR)
		{
			if (Vars.Immediate & 3)
			{
				QueueError(ERROR_ERROR,"PC relative address must be word aligned");
				Global.RamPos += OpcodeSize;
				return false;
			}

			int pos = Vars.Immediate-((Global.RamPos+4) & 0xFFFFFFFD);
			if (pos < 0 || pos > 1020)
			{
				QueueError(ERROR_ERROR,"PC relative address out of range");
				return false;
			}
			Vars.Immediate = pos >> 2;
		}

		if (Vars.ImmediateBitLen != 32)
		{
			if (Vars.Immediate >= (1 << Vars.ImmediateBitLen))
			{
				QueueError(ERROR_ERROR,"Immediate value %X out of range",Vars.Immediate);
				return false;
			}
			Vars.Immediate &= (1 << Vars.ImmediateBitLen)-1;
		}
	}

	Global.RamPos += OpcodeSize;
	return false;
}

void CThumbInstruction::WriteInstruction(unsigned short encoding)
{
	if (Global.Output.write(&encoding,2) == -1)
	{
		PrintError(ERROR_ERROR,"No file opened");
	}
}

void CThumbInstruction::Encode()
{
	unsigned int encoding = Opcode.encoding;;

	if (Opcode.type == THUMB_TYPE19)	// THUMB.19: long branch with link
	{
		encoding |= ((Vars.Immediate >> 11) & 0x7FF);
		WriteInstruction(encoding);

		if (Opcode.flags & THUMB_EXCHANGE)
		{
			WriteInstruction(0xE800 | (Vars.Immediate & 0x7FF));
		} else {
			WriteInstruction(0xF800 | (Vars.Immediate & 0x7FF));
		}
	} else {
		switch (Opcode.type)
		{
		case THUMB_TYPE1:	// THUMB.1: move shifted register
			encoding |= (Vars.Immediate << 6);
			encoding |= (Vars.rs.Number << 3);
			encoding |= (Vars.rd.Number << 0);
			break;
		case THUMB_TYPE2:	// THUMB.2: add/subtract
			if (Opcode.flags & THUMB_IMMEDIATE)
			{
				encoding |= (Vars.Immediate << 6);
			} else if (Opcode.flags & THUMB_REGISTER)
			{
				encoding |= (Vars.rn.Number << 6);
			}
			encoding |= (Vars.rs.Number << 3);
			encoding |= (Vars.rd.Number << 0);
			break;
		case THUMB_TYPE3:	// THUMB.3: move/compare/add/subtract immediate
			encoding |= (Vars.rd.Number << 8);
			encoding |= (Vars.Immediate << 0);
			break;
		case THUMB_TYPE4:	// THUMB.4: ALU operations
			encoding |= (Vars.rs.Number << 3);
			encoding |= (Vars.rd.Number << 0);
			break;
		case THUMB_TYPE5:	// THUMB.5: Hi register operations/branch exchange
			if (Opcode.flags & THUMB_D)
			{
				if (Vars.rd.Number > 0x7) encoding |= (1 << 7);
				encoding |= (Vars.rd.Number & 0x7);
			}
			if (Opcode.flags & THUMB_S)
			{
				if (Vars.rs.Number > 0x7) encoding |= (1 << 6);
				encoding |= ((Vars.rs.Number & 0x7) << 3);
			}
			break;
		case THUMB_TYPE6:	// THUMB.6: load PC-relative
			encoding |= (Vars.rd.Number << 8);
			encoding |= (Vars.Immediate << 0);
			break;
		case THUMB_TYPE7:	// THUMB.7: load/store with register offset
		case THUMB_TYPE8:	// THUMB.8: load/store sign-extended byte/halfword
			encoding |= (Vars.ro.Number << 6);
			encoding |= (Vars.rs.Number << 3);
			encoding |= (Vars.rd.Number << 0);
			break;
		case THUMB_TYPE9:	// THUMB.9: load/store with immediate offset
		case THUMB_TYPE10:	// THUMB.10: load/store halfword
			if (Opcode.flags & THUMB_IMMEDIATE) encoding |= (Vars.Immediate << 6);
			encoding |= (Vars.rs.Number << 3);
			encoding |= (Vars.rd.Number << 0);
			break;
		case THUMB_TYPE11:	// THUMB.11: load/store SP-relative
			encoding |= (Vars.rd.Number << 8);
			if (Opcode.flags & THUMB_IMMEDIATE) encoding |= (Vars.Immediate << 0);
			break;
		case THUMB_TYPE12:	// THUMB.12: get relative address
			encoding |= (Vars.rd.Number << 8);
			encoding |= (Vars.Immediate << 0);
			break;
		case THUMB_TYPE13:	// THUMB.13: add offset to stack pointer
			if (Vars.Immediate & 0x80)	// sub
			{
				encoding |= 1 << 7;
				Vars.Immediate = 0x100-Vars.Immediate;
			}
			encoding |= (Vars.Immediate << 0);
			break;
		case THUMB_TYPE14:	// THUMB.14: push/pop registers
			if (Vars.rlist & 0xC000) encoding |= (1 << 8); // r14 oder r15
			encoding |= (Vars.rlist & 0xFF);
			break;
		case THUMB_TYPE15:	// THUMB.15: multiple load/store
			encoding |= (Vars.rd.Number << 8);
			encoding |= (Vars.rlist & 0xFF);
			break;
		case THUMB_TYPE16:	// THUMB.16: conditional branch
		case THUMB_TYPE17:	// THUMB.17: software interrupt and breakpoint
		case THUMB_TYPE18:	// THUMB.18: unconditional branch
			encoding |= (Vars.Immediate << 0);
			break;
		}
		WriteInstruction(encoding);
	}
}

void CThumbInstruction::FormatInstruction(char* encoding,tThumbOpcodeVariables& Vars, char* dest)
{
	while (*encoding != 0)
	{
		switch (*encoding)
		{
		case 'D':
		case 'd':
			dest += sprintf(dest,"%s",Vars.rd.Name);
			encoding++;
			break;
		case 'S':
		case 's':
			dest += sprintf(dest,"%s",Vars.rs.Name);
			encoding++;
			break;
		case 'n':
			dest += sprintf(dest,"%s",Vars.rn.Name);
			encoding++;
			break;
		case 'o':
			dest += sprintf(dest,"%s",Vars.ro.Name);
			encoding++;
			break;
		case 'I':
			dest += sprintf(dest,"0x%0*X",(Vars.ImmediateBitLen+3)>>2,Vars.OriginalImmediate);
			encoding += 2;
			break;
		case 'i':
			dest += sprintf(dest,"0x%0*X",(Vars.ImmediateBitLen+3)>>2,Vars.OriginalImmediate & ((1 << Vars.ImmediateBitLen)-1));
			encoding += 2;
			break;
		case 'r':	// forced register
			dest += sprintf(dest,"r%d",*(encoding+1));
			encoding += 2;
			break;
		case 'R':
			dest += sprintf(dest,"%s",Vars.RlistStr);
			encoding += 3;
			break;
		case '/':	// optional
			encoding += 2;
			break;
		default:
			*dest++ = *encoding++;
			break;
		}
	}
	*dest = 0;
}

void CThumbInstruction::WriteTempData(FILE *&Output)
{
	char str[256];

	int pos = sprintf(str,"   %s",Opcode.name);
	while (pos < 11) str[pos++] = ' ';
	str[pos] = 0;
	FormatInstruction(Opcode.mask,Vars,&str[pos]);

	WriteToTempData(Output,str,RamPos);
}