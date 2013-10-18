#include "stdafx.h"
#include "CMipsInstruction.h"
#include "Core/Common.h"
#include "Mips.h"
#include "MipsOpcodes.h"
#include "Core/FileManager.h"

CMipsInstruction::CMipsInstruction()
{
	subInstruction = NULL;
	Loaded = false;
	IgnoreLoadDelay = Mips.GetIgnoreDelay();
}

CMipsInstruction::~CMipsInstruction()
{
	if (subInstruction != NULL)
		delete subInstruction;
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
				if (MipsGetFloatRegister(Line,RetLen,vars.rt) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'D':	// float reg
				if (MipsGetFloatRegister(Line,RetLen,vars.rd) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'S':	// float reg
				if (MipsGetFloatRegister(Line,RetLen,vars.rs) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 't':
				if (MipsGetRegister(Line,RetLen,vars.rt) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'd':
				if (MipsGetRegister(Line,RetLen,vars.rd) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 's':
				if (MipsGetRegister(Line,RetLen,vars.rs) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'a':	// 5 bit immediate
			case 'i':	// 16 bit immediate
			case 'b':	// 20 bit immediate
			case 'I':	// 32 bit immediate
				if (MipsCheckImmediate(Line,immediate.expression,RetLen) == false) return false;
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

	immediateType = MIPS_NOIMMEDIATE;
	if (immediate.expression.isLoaded())
	{
		if (immediate.expression.check() == false)
		{
			NoCheckError = true;
			return false;
		}

		if (Opcode.flags & O_I5)
			immediateType = MIPS_IMMEDIATE5;
		else if (Opcode.flags & O_I16)
			immediateType = MIPS_IMMEDIATE16;
		else if (Opcode.flags & O_I20)
			immediateType = MIPS_IMMEDIATE20;
		else if (Opcode.flags & O_I26)
			immediateType = MIPS_IMMEDIATE26;
		else
		{
			Logger::printError(Logger::Error,L"Unknown immediate type");
			return false;
		}
	}

	setOmittedRegisters();
	return true;
}

void CMipsInstruction::setOmittedRegisters()
{
	// copy over omitted registers
	if ((Opcode.flags & O_RSD) == O_RSD)
		vars.rd = vars.rs;
	
	if ((Opcode.flags & O_RST) == O_RST)
		vars.rt = vars.rs;
	
	if ((Opcode.flags & O_RDT) == O_RDT)
		vars.rt = vars.rd;
	
	if ((Opcode.flags & MO_FRSD) == MO_FRSD)
		vars.rd = vars.rs;
	
	if ((Opcode.flags & MO_FRST) == MO_FRST)
		vars.rt = vars.rs;
	
	if ((Opcode.flags & MO_FRDT) == MO_FRDT)
		vars.rt = vars.rd;
}

void CMipsInstruction::FormatInstruction(char* encoding,MipsOpcodeVariables& Vars, char* dest)
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
			dest += sprintf(dest,"%s",vars.rs.Name);
			encoding++;
			break;
		case 'd':
		case 'D':
			dest += sprintf(dest,"%s",vars.rd.Name);
			encoding++;
			break;
		case 't':
		case 'T':
			dest += sprintf(dest,"%s",vars.rt.Name);
			encoding++;
			break;
		case 'i':	// 16 bit immediate
			dest += sprintf(dest,"0x%X",immediate.originalValue & 0xFFFF);
			encoding++;
			break;
		case 'I':	// 32 bit immediate
			dest += sprintf(dest,"0x%X",immediate.originalValue);
			encoding++;
			break;
		case 'a':	// 5 bit immediate
			dest += sprintf(dest,"0x%X",immediate.originalValue & 0x1F);
			encoding++;
			break;
		case 'b':	// 26 bit immediate
			dest += sprintf(dest,"0x%X",immediate.originalValue & 0x3FFFFFF);
			encoding++;
			break;
		default:
			*dest++ = *encoding++;
			break;
		}
	}
	*dest = 0;
}

int getImmediateBits(MipsImmediateType type)
{
	switch (type)
	{
	case MIPS_IMMEDIATE5:
		return 5;
	case MIPS_IMMEDIATE16:
		return 16;
	case MIPS_IMMEDIATE20:
		return 20;
	case MIPS_IMMEDIATE26:
		return 26;
	default:
		return 0;
	}
}

bool CMipsInstruction::Validate()
{
	CStringList List;
	bool Result = false;

	if (subInstruction != NULL)
		subInstruction->Validate();

	RamPos = g_fileManager->getVirtualAddress();
	if (RamPos % 4)
	{
		Logger::queueError(Logger::Error,L"opcode not aligned to word boundary");
		return false;
	}

	// check immediates
	if (immediateType != MIPS_NOIMMEDIATE)
	{
		if (immediate.expression.evaluate(immediate.value,true) == false)
			return false;

		immediate.originalValue = immediate.value;
		
		if (Opcode.flags & O_IPCA)	// absolute value >> 2)
		{
			immediate.value = (immediate.value >> 2) & 0x3FFFFFF;
		} else if (Opcode.flags & O_IPCR)	// relative 16 bit value
		{
			int num = (immediate.value-RamPos-4);
			
			if (num > 0x20000 || num < (-0x20000))
			{
				Logger::queueError(Logger::Error,L"Branch target %08X out of range",immediate.value);
				return false;
			}
			immediate.value = num >> 2;
		}
		
		int immediateBits = getImmediateBits(immediateType);
		unsigned int mask = (0xFFFFFFFF << (32-immediateBits)) >> (32-immediateBits);
		int digits = (immediateBits+3) / 4;

		if (std::abs(immediate.value) > mask)
		{
			Logger::queueError(Logger::Error,L"Immediate value %0*X out of range",digits,immediate.value);
			return false;
		}

		immediate.value &= mask;
	}

	// check load delay
	if (Mips.hasLoadDelay() && Mips.GetLoadDelay() && IgnoreLoadDelay == false && !(Opcode.flags & MO_IGNORERTD))
	{
		bool fix = false;

		if ((Opcode.flags & O_RD) && vars.rd.Number == Mips.GetLoadDelayRegister())
		{
			Logger::queueError(Logger::Warning,L"register %S may not be available due to load delay",vars.rd.Name);
			fix = true;
		} else if ((Opcode.flags & O_RS) && vars.rs.Number == Mips.GetLoadDelayRegister())
		{
			Logger::queueError(Logger::Warning,L"register %S may not be available due to load delay",vars.rs.Name);
			fix = true;
		} else if ((Opcode.flags & O_RT) && vars.rt.Number == Mips.GetLoadDelayRegister())
		{
			Logger::queueError(Logger::Warning,L"register %S may not be available due to load delay",vars.rt.Name);
			fix = true;
		}

		if (Mips.GetFixLoadDelay() == true && fix == true)
		{
			if (subInstruction == NULL)
			{
				subInstruction = new CMipsInstruction();
				subInstruction->Load("nop","");
			}

			Result = true;
			subInstruction->Validate();
			RamPos = g_fileManager->getVirtualAddress();

			Logger::printError(Logger::Notice,L"added nop to ensure correct behavior");
		}
	}

	if ((Opcode.flags & MO_NODELAY) && Mips.GetDelaySlot() == true && IgnoreLoadDelay == false)
	{
		Logger::queueError(Logger::Error,L"This instruction can't be in a delay slot");
	}

	Mips.SetDelaySlot(Opcode.flags & MO_DELAY ? true : false);

	// now check if this opcode causes a load delay
	if (Mips.GetVersion() & MARCH_PSX)
		Mips.SetLoadDelay(Opcode.flags & MO_DELAYRT ? true : false,vars.rt.Number);
	
	g_fileManager->advanceMemory(4);
	return Result;
}


void CMipsInstruction::Encode()
{
	if (subInstruction != NULL)
		subInstruction->Encode();
	int encoding = Opcode.destencoding;

	if (Opcode.flags & O_RS) encoding |= (vars.rs.Number << 21);	// source reg
	if (Opcode.flags & O_RT) encoding |= (vars.rt.Number << 16);	// target reg
	if (Opcode.flags & O_RD) encoding |= (vars.rd.Number << 11);	// dest reg

	if (Opcode.flags & MO_FRT) encoding |= (vars.rt.Number << 16);	// float target
	if (Opcode.flags & MO_FRS) encoding |= (vars.rs.Number << 11);	// float source
	if (Opcode.flags & MO_FRD) encoding |= (vars.rd.Number << 6);	// float dest

	switch (immediateType)
	{
	case MIPS_IMMEDIATE5:
	case MIPS_IMMEDIATE20:
		encoding |= immediate.value << 6;
		break;
	case MIPS_IMMEDIATE16:
	case MIPS_IMMEDIATE26:
		encoding |= immediate.value;
		break;
	}
	
	g_fileManager->write(&encoding,4);
}

void CMipsInstruction::writeTempData(TempData& tempData)
{
	char str[256];

	if (subInstruction != NULL)
		subInstruction->writeTempData(tempData);

	int pos = sprintf(str,"   %s",Opcode.name);
	while (pos < 11) str[pos++] = ' ';
	str[pos] = 0;
	FormatInstruction(Opcode.encoding,vars,&str[pos]);

	tempData.writeLine(RamPos,convertUtf8ToWString(str));
}
