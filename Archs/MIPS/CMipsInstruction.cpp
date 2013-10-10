#include "stdafx.h"
#include "CMipsInstruction.h"
#include "Core/Common.h"
#include "Mips.h"
#include "MipsOpcodes.h"

CMipsInstruction::CMipsInstruction()
{
	SubInstructionEnabled = false;
	Loaded = false;
	IgnoreLoadDelay = Mips.GetIgnoreDelay();
}

CMipsInstruction::~CMipsInstruction()
{
	if (SubInstructionEnabled == true) delete SubInstruction;
}

bool CMipsInstruction::Load(char* Name, char* Params)
{
	bool paramfail = false;
	NoCheckError = false;

	for (int z = 0; MipsOpcodes[z].name != NULL; z++)
	{
		if (MipsOpcodes[z].ver & Mips.GetVersion())
		{
			if (strcmp(Name,MipsOpcodes[z].name) == 0)
			{
				if (LoadEncoding(MipsOpcodes[z],Params) == true)
				{
					Loaded = true;
					return true;
				}
				paramfail = true;
			}
		}
	}

	if (NoCheckError == false)
	{
		if (paramfail == true)
		{
			Logger::printError(Logger::Error,L"MIPS parameter failure \"%S\"",Params);
		} else {
			Logger::printError(Logger::Error,L"Invalid MIPS opcode \"%S\"",Name);
		}
	}
	return false;
}



bool CMipsInstruction::LoadEncoding(const tMipsOpcode& SourceOpcode, char* Line)
{
	char ImmediateBuffer[512];

	int RetLen;
	CStringList List;
	bool Immediate = false;

	char* SourceEncoding = SourceOpcode.encoding;
	char* OriginalLine = Line;

	while (*Line == ' ' || *Line == '\t') Line++;

	if (!(*SourceEncoding == 0 && *Line == 0))
	{
		while (*SourceEncoding != NULL)
		{
			while (*Line == ' ' || *Line == '\t') Line++;
			if (*Line == 0) return false;

			switch (*SourceEncoding)
			{
			case 'T':	// float reg
				if (MipsGetFloatRegister(Line,RetLen,Vars.rt) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'D':	// float reg
				if (MipsGetFloatRegister(Line,RetLen,Vars.rd) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'S':	// float reg
				if (MipsGetFloatRegister(Line,RetLen,Vars.rs) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 't':
				if (MipsGetRegister(Line,RetLen,Vars.rt) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'd':
				if (MipsGetRegister(Line,RetLen,Vars.rd) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 's':
				if (MipsGetRegister(Line,RetLen,Vars.rs) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'i':	// 16 bit immediate
			case 'I':	// 32 bit immediate
			case 'a':	// 5 bit immediate
			case 'b':	// 20 bit immediate
				if (MipsCheckImmediate(Line,ImmediateBuffer,RetLen,List) == false) return false;
				Immediate = true;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'r':	// forced register
				if (MipsGetRegister(Line,RetLen) != *(SourceEncoding+1)) return false;
				Line += RetLen;
				SourceEncoding += 2;
				break;
			default:	// everything else
				if (*SourceEncoding++ != *Line++) return false;
				break;
			}
		}
	}

	while (*Line == ' ' || *Line == '\t') Line++;
	if (*Line != 0)	return false;	// there's something else, bad
	
	// opcode is ok - now set all flags
	Opcode = SourceOpcode;

	if (Immediate == true)
	{
		if (CheckPostfix(List,true) == false)
		{
			Logger::printError(Logger::Error,L"Invalid expression \"%S\"",ImmediateBuffer);
			NoCheckError = true;
			return false;
		}

		if (Opcode.flags & O_I5)
		{
			Vars.ImmediateType = MIPS_IMMEDIATE5;
		} else if (Opcode.flags & O_I16)
		{
			Vars.ImmediateType = MIPS_IMMEDIATE16;
		} else if (Opcode.flags & O_I20)
		{
			Vars.ImmediateType = MIPS_IMMEDIATE20;
		} else if (Opcode.flags & O_I26)
		{
			Vars.ImmediateType = MIPS_IMMEDIATE26;
		}
		Vars.ImmediateExpression.Load(List);
	} else {
		Vars.ImmediateType = MIPS_NOIMMEDIATE;
	}

	return true;
}

void CMipsInstruction::FormatInstruction(char* encoding,tMipsOpcodeVariables& Vars, char* dest)
{
	while (*encoding != 0)
	{
		switch (*encoding)
		{
		case 'r':	// forced register
			dest += sprintf(dest,"r%d",*(encoding+1));
			encoding+=2;
			break;
		case 's':
		case 'S':
			dest += sprintf(dest,"%s",Vars.rs.Name);
			encoding++;
			break;
		case 'd':
		case 'D':
			dest += sprintf(dest,"%s",Vars.rd.Name);
			encoding++;
			break;
		case 't':
		case 'T':
			dest += sprintf(dest,"%s",Vars.rt.Name);
			encoding++;
			break;
		case 'i':	// 16 bit immediate
			dest += sprintf(dest,"0x%X",Vars.OriginalImmediate & 0xFFFF);
			encoding++;
			break;
		case 'I':	// 32 bit immediate
			dest += sprintf(dest,"0x%X",Vars.OriginalImmediate);
			encoding++;
			break;
		case 'a':	// 5 bit immediate
			dest += sprintf(dest,"0x%X",Vars.OriginalImmediate & 0x1F);
			encoding++;
			break;
		case 'b':	// 26 bit immediate
			dest += sprintf(dest,"0x%X",Vars.OriginalImmediate & 0x3FFFFFF);
			encoding++;
			break;
		default:
			*dest++ = *encoding++;
			break;
		}
	}
	*dest = 0;
}

void CMipsInstruction::WriteInstruction(unsigned int encoding)
{
	if (Global.Output.write(&encoding,4) == -1)
	{
		Logger::printError(Logger::Error,L"No file opened");
	}
}

bool CMipsInstruction::Validate()
{
	CStringList List;
	bool Result = false;

	if (SubInstructionEnabled == true) SubInstruction->Validate();

	RamPos = Global.RamPos;

	if (RamPos % 4)
	{
		Logger::queueError(Logger::Warning,L"opcode not aligned to word boundary");
	}

	// check immediates
	if (Vars.ImmediateType != MIPS_NOIMMEDIATE)
	{
		if (ParsePostfix(Vars.ImmediateExpression,&List,Vars.Immediate) == false)
		{
			if (List.GetCount() == 0)
			{
				Logger::queueError(Logger::Error,L"Invalid expression");
			} else {
				for (int l = 0; l < List.GetCount(); l++)
				{
					Logger::queueError(Logger::Error,convertUtf8ToWString(List.GetEntry(l)));
				}
			}
			return false;
		}
		Vars.OriginalImmediate = Vars.Immediate;
 
		if (Opcode.flags & O_IPCA)	// absolute value >> 2)
		{
			Vars.Immediate = (Vars.Immediate >> 2) & 0x3FFFFFF;
		} else if (Opcode.flags & O_IPCR)	// relativer 16 bit wert
		{
			int num = (Vars.Immediate-RamPos-4);
			
			if (num > 0x20000 || num < (-0x20000))
			{
				Logger::queueError(Logger::Error,L"Branch target %08X out of range",Vars.Immediate);
				return false;
			}
			Vars.Immediate = num >> 2;
		}

		switch (Vars.ImmediateType)
		{
		case MIPS_IMMEDIATE5:
			if (Vars.Immediate > 0x1F)
			{
				Logger::queueError(Logger::Error,L"Immediate value %02X out of range",Vars.OriginalImmediate);
				return false;
			}
			break;
			Vars.Immediate &= 0x1F;
			break;
		case MIPS_IMMEDIATE16:
			if (abs(Vars.Immediate) > 0xFFFF)
			{
				Logger::queueError(Logger::Error,L"Immediate value %04X out of range",Vars.OriginalImmediate);
				return false;
			}
			Vars.Immediate &= 0xFFFF;
			break;
		case MIPS_IMMEDIATE20:
			if (abs(Vars.Immediate) > 0xFFFFF)
			{
				Logger::queueError(Logger::Error,L"Immediate value %08X out of range",Vars.OriginalImmediate);
				return false;
			}
			Vars.Immediate &= 0xFFFFF;
			break;
		case MIPS_IMMEDIATE26:
			if (abs(Vars.Immediate) > 0x3FFFFFF)
			{
				Logger::queueError(Logger::Error,L"Immediate value %08X out of range",Vars.OriginalImmediate);
				return false;
			}
			Vars.Immediate &= 0x3FFFFFF;
			break;
		}
	}

	// check load delay
	if (Mips.GetLoadDelay() && IgnoreLoadDelay == false && !(Opcode.flags & MO_IGNORERTD))
	{
		bool fix = false;

		if ((Opcode.flags & O_RD) && Vars.rd.Number == Mips.GetLoadDelayRegister()
			|| (Opcode.flags & O_RDT) && Vars.rd.Number == Mips.GetLoadDelayRegister())
		{
			Logger::queueError(Logger::Warning,L"register %s may not be available due to load delay",Vars.rd.Name);
			fix = true;
		} else if ((Opcode.flags & O_RS) && Vars.rs.Number == Mips.GetLoadDelayRegister()
			|| (Opcode.flags & O_RSD) && Vars.rs.Number == Mips.GetLoadDelayRegister()
			|| (Opcode.flags & O_RST) && Vars.rs.Number == Mips.GetLoadDelayRegister())
		{
			Logger::queueError(Logger::Warning,L"register %s may not be available due to load delay",Vars.rs.Name);
			fix = true;
		} else if ((Opcode.flags & O_RT) && Vars.rt.Number == Mips.GetLoadDelayRegister())
		{
			Logger::queueError(Logger::Warning,L"register %s may not be available due to load delay",Vars.rt.Name);
			fix = true;
		}

		if (Mips.GetFixLoadDelay() == true && fix == true)
		{
			SubInstruction = new CMipsInstruction();
			SubInstruction->Load("nop","");
			SubInstructionEnabled = true;
			Result = true;
			SubInstruction->Validate();
			RamPos = Global.RamPos;

			Logger::printError(Logger::Notice,L"added nop to ensure correct behavior");
		}
	}

	if ((Opcode.flags & MO_NODELAY) && Mips.GetDelaySlot() == true && IgnoreLoadDelay == false)
	{
		Logger::queueError(Logger::Error,L"This instruction can't be in a delay slot");
	}

	Mips.SetDelaySlot(Opcode.flags & MO_DELAY ? true : false);

	// now check if this opcode causes a load delay
	Mips.SetLoadDelay(Opcode.flags & MO_DELAYRT ? true : false,Vars.rt.Number);
	
	Global.RamPos += 4;
	return Result;
}


void CMipsInstruction::Encode()
{
	if (SubInstructionEnabled == true) SubInstruction->Encode();
	int encoding = Opcode.destencoding;

	if (Opcode.flags & O_RS) encoding |= (Vars.rs.Number << 21);	// source reg
	if (Opcode.flags & O_RT) encoding |= (Vars.rt.Number << 16);	// target reg
	if (Opcode.flags & O_RD) encoding |= (Vars.rd.Number << 11);	// dest reg
	if (Opcode.flags & O_RSD)	// s = d & s
	{
		encoding |= (Vars.rs.Number << 21);
		encoding |= (Vars.rs.Number << 11);
	}
	if (Opcode.flags & O_RST)	// s = t & s
	{
		encoding |= (Vars.rs.Number << 21);
		encoding |= (Vars.rs.Number << 16);
	}
	if (Opcode.flags & O_RDT)	// d = t & d
	{
		encoding |= (Vars.rd.Number << 16);
		encoding |= (Vars.rd.Number << 11);
	}

	if (Opcode.flags & MO_FRT) encoding |= (Vars.rt.Number << 16);	// float target
	if (Opcode.flags & MO_FRS) encoding |= (Vars.rs.Number << 11);	// float source
	if (Opcode.flags & MO_FRD) encoding |= (Vars.rd.Number << 6);	// float dest
	if (Opcode.flags & MO_FRSD)	// s = d & s
	{
		encoding |= (Vars.rs.Number << 6);
		encoding |= (Vars.rs.Number << 11);
	}
	if (Opcode.flags & MO_FRST)	// s = t & s
	{
		encoding |= (Vars.rs.Number << 11);
		encoding |= (Vars.rs.Number << 16);
	}
	if (Opcode.flags & MO_FRDT)	// d = t & d
	{
		encoding |= (Vars.rd.Number << 6);
		encoding |= (Vars.rd.Number << 16);
	}

	switch (Vars.ImmediateType)
	{
	case MIPS_IMMEDIATE5:
		encoding |= ((Vars.Immediate & 0x1F) << 6);
		break;
	case MIPS_IMMEDIATE16:
		encoding |= (Vars.Immediate & 0xFFFF);
		break;
	case MIPS_IMMEDIATE20:
		encoding |= (Vars.Immediate & 0xFFFFF) << 6;
		break;
	case MIPS_IMMEDIATE26:
		encoding |= Vars.Immediate & 0x3FFFFFF;
		break;
	}

	WriteInstruction(encoding);
}

void CMipsInstruction::writeTempData(TempData& tempData)
{
	char str[256];

	if (SubInstructionEnabled == true) SubInstruction->writeTempData(tempData);

	int pos = sprintf(str,"   %s",Opcode.name);
	while (pos < 11) str[pos++] = ' ';
	str[pos] = 0;
	FormatInstruction(Opcode.encoding,Vars,&str[pos]);

	tempData.writeLine(RamPos,convertUtf8ToWString(str));
}
