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

	const MipsArchDefinition& arch = mipsArchs[Mips.GetVersion()];
	for (int z = 0; MipsOpcodes[z].name != NULL; z++)
	{
		if ((MipsOpcodes[z].archs & arch.supportSets) == 0)
			continue;
		if ((MipsOpcodes[z].archs & arch.excludeMask) != 0)
			continue;

		if ((MipsOpcodes[z].flags & MO_64BIT) && !(arch.flags & MO_64BIT))
			continue;
		if ((MipsOpcodes[z].flags & MO_FPU) && !(arch.flags & MO_FPU))
			continue;

		if (parseOpcode(MipsOpcodes[z],Name) == true)
		{
			if (LoadEncoding(MipsOpcodes[z],Params) == true)
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
			Logger::printError(Logger::Error,L"MIPS parameter failure \"%S\"",Params);
		} else {
			Logger::printError(Logger::Error,L"Invalid MIPS %S opcode \"%S\"",arch.name,Name);
		}
	}
	return false;
}


bool CMipsInstruction::parseOpcode(const tMipsOpcode& SourceOpcode, char* Line)
{
	vfpuSize = -1;

	char* SourceEncoding = SourceOpcode.name;
	while (*SourceEncoding != NULL)
	{
		if (*Line == 0) return false;

		switch (*SourceEncoding)
		{
		case 'S':	// vfpu size
			switch (*Line)
			{
			case 's':
				vfpuSize = 0;
				break;
			case 'p':
				vfpuSize = 1;
				break;
			case 't':
				vfpuSize = 2;
				break;
			case 'q':
				vfpuSize = 3;
				break;
			default:
				return false;
			}
			SourceEncoding++;
			Line++;
			break;
		default:
			if (*SourceEncoding++ != *Line++) return false;
			break;
		}
	}
	
	if (*Line != 0)	return false;	// there's something else, bad
	return true;
}

bool CMipsInstruction::LoadEncoding(const tMipsOpcode& SourceOpcode, char* Line)
{
	int RetLen;
	CStringList List;
	bool Immediate = false;
	
	immediateType = MIPS_NOIMMEDIATE;
	registers.reset();

	if (vfpuSize == -1)
	{
		if (SourceOpcode.flags & MO_VFPU_SINGLE)
			vfpuSize = 0;
		else if (SourceOpcode.flags & MO_VFPU_QUAD)
			vfpuSize = 3;
	}

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
				if (MipsGetFloatRegister(Line,RetLen,registers.frt) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'D':	// float reg
				if (MipsGetFloatRegister(Line,RetLen,registers.frd) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'S':	// float reg
				if (MipsGetFloatRegister(Line,RetLen,registers.frs) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 't':
				if (MipsGetRegister(Line,RetLen,registers.grt) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'd':
				if (MipsGetRegister(Line,RetLen,registers.grd) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 's':
				if (MipsGetRegister(Line,RetLen,registers.grs) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'v':	// vfpu vector register
				switch (*(SourceEncoding+1))
				{
				case 's':
					if (MipsGetVFPURegister(Line,registers.vrs,vfpuSize) == false) return false;
					if (registers.vrs.type != MIPSVFPU_VECTOR) return false;
					if ((SourceOpcode.flags & MO_VFPU_6BIT) && (registers.vrs.num & 0x40)) return false;
					break;
				case 't':
					if (MipsGetVFPURegister(Line,registers.vrt,vfpuSize) == false) return false;
					if (registers.vrt.type != MIPSVFPU_VECTOR) return false;
					if ((SourceOpcode.flags & MO_VFPU_6BIT) && (registers.vrt.num & 0x40)) return false;
					break;
				case 'd':
					if (MipsGetVFPURegister(Line,registers.vrd,vfpuSize) == false) return false;
					if (registers.vrd.type != MIPSVFPU_VECTOR) return false;
					if ((SourceOpcode.flags & MO_VFPU_6BIT) && (registers.vrd.num & 0x40)) return false;
					break;
				default:
					return false;
				}
				Line += 4;
				SourceEncoding += 2;
				break;
			case 'a':	// 5 bit immediate
				if (MipsCheckImmediate(Line,immediate.expression,RetLen) == false) return false;
				immediateType = MIPS_IMMEDIATE5;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'i':	// 16 bit immediate
				if (MipsCheckImmediate(Line,immediate.expression,RetLen) == false) return false;
				immediateType = MIPS_IMMEDIATE16;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'b':	// 20 bit immediate
				if (MipsCheckImmediate(Line,immediate.expression,RetLen) == false) return false;
				immediateType = MIPS_IMMEDIATE20;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'I':	// 32 bit immediate
				if (MipsCheckImmediate(Line,immediate.expression,RetLen) == false) return false;
				immediateType = MIPS_IMMEDIATE26;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'r':	// forced register
				if (MipsGetRegister(Line,RetLen) != *(SourceEncoding+1)) return false;
				Line += RetLen;
				SourceEncoding += 2;
				break;
			case '/':	// forced letter
				SourceEncoding++;	// fallthrough
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
	if (immediate.expression.isLoaded())
	{
		if (immediate.expression.check() == false)
		{
			NoCheckError = true;
			return false;
		}
	}

	setOmittedRegisters();
	return true;
}

void CMipsInstruction::setOmittedRegisters()
{
	// copy over omitted registers
	if (Opcode.flags & MO_RSD)
		registers.grd = registers.grs;
	
	if (Opcode.flags & MO_RST)
		registers.grt = registers.grs;
	
	if (Opcode.flags & MO_RDT)
		registers.grt = registers.grd;
	
	if (Opcode.flags & MO_FRSD)
		registers.frd = registers.frs;
}

int CMipsInstruction::formatOpcodeName(char* dest)
{
	char* encoding = Opcode.name;
	char* start = dest;

	while (*encoding != 0)
	{
		switch (*encoding)
		{
		case 'S':
			*dest++ = "sptq"[vfpuSize];
			encoding++;
			break;
		default:
			*dest++ = *encoding++;
			break;
		}
	}
	*dest = 0;
	return dest-start;
}

void CMipsInstruction::formatParameters(char* dest)
{
	char* encoding = Opcode.encoding;

	while (*encoding != 0)
	{
		switch (*encoding)
		{
		case 'r':	// forced register
			dest += sprintf(dest,"r%d",*(encoding+1));
			encoding+=2;
			break;
		case 's':
			dest += sprintf(dest,"%s",registers.grs.name);
			encoding++;
			break;
		case 'S':
			dest += sprintf(dest,"%s",registers.frs.name);
			encoding++;
			break;
		case 'd':
			dest += sprintf(dest,"%s",registers.grd.name);
			encoding++;
			break;
		case 'D':
			dest += sprintf(dest,"%s",registers.frd.name);
			encoding++;
			break;
		case 't':
			dest += sprintf(dest,"%s",registers.grt.name);
			encoding++;
			break;
		case 'T':
			dest += sprintf(dest,"%s",registers.frt.name);
			encoding++;
			break;
		case 'v':
			switch (*(encoding+1))
			{
			case 'd':
				dest += sprintf(dest,"%s",registers.vrd.name);
				break;
			case 's':
				dest += sprintf(dest,"%s",registers.vrs.name);
				break;
			case 't':
				dest += sprintf(dest,"%s",registers.vrt.name);
				break;
			}
			encoding += 2;
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
		
		if (Opcode.flags & MO_IMMALIGNED)	// immediate must be aligned
		{
			if (immediate.value % 4)
			{
				Logger::queueError(Logger::Error,L"Immediate must be word aligned",immediate.value);
				return false;
			}
		}

		if (Opcode.flags & MO_IPCA)	// absolute value >> 2)
		{
			immediate.value = (immediate.value >> 2) & 0x3FFFFFF;
		} else if (Opcode.flags & MO_IPCR)	// relative 16 bit value
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

		if ((unsigned int)std::abs(immediate.value) > mask)
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

		if (registers.grd.num != -1 && registers.grd.num == Mips.GetLoadDelayRegister())
		{
			Logger::queueError(Logger::Warning,L"register %S may not be available due to load delay",registers.grd.name);
			fix = true;
		} else if (registers.grs.num != -1 && registers.grs.num == Mips.GetLoadDelayRegister())
		{
			Logger::queueError(Logger::Warning,L"register %S may not be available due to load delay",registers.grs.name);
			fix = true;
		} else if (registers.grt.num != -1 && registers.grt.num == Mips.GetLoadDelayRegister())
		{
			Logger::queueError(Logger::Warning,L"register %S may not be available due to load delay",registers.grt.name);
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

	if ((Opcode.flags & MO_NODELAYSLOT) && Mips.GetDelaySlot() == true && IgnoreLoadDelay == false)
	{
		Logger::queueError(Logger::Error,L"This instruction can't be in a delay slot");
	}

	Mips.SetDelaySlot(Opcode.flags & MO_DELAY ? true : false);

	// now check if this opcode causes a load delay
	if (Mips.hasLoadDelay())
		Mips.SetLoadDelay(Opcode.flags & MO_DELAYRT ? true : false,registers.grt.num);
	
	g_fileManager->advanceMemory(4);
	return Result;
}

void CMipsInstruction::encodeNormal()
{
	int encoding = Opcode.destencoding;

	if (registers.grs.num != -1) encoding |= (registers.grs.num << 21);	// source reg
	if (registers.grt.num != -1) encoding |= (registers.grt.num << 16);	// target reg
	if (registers.grd.num != -1) encoding |= (registers.grd.num << 11);	// dest reg
	
	if (registers.frt.num != -1) encoding |= (registers.frt.num << 16);	// float target reg
	if (registers.frs.num != -1) encoding |= (registers.frs.num << 21);	// float source reg
	if (registers.frd.num != -1) encoding |= (registers.frd.num << 6);	// float dest reg

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

	if (Opcode.flags & MO_VFPU_MIXED)
	{
		// always vrt
		encoding |= registers.vrt.num >> 5;
		encoding |= (registers.vrt.num & 0x1F) << 16;
	}
	
	g_fileManager->write(&encoding,4);
}

void CMipsInstruction::encodeVfpu()
{
	int encoding = Opcode.destencoding;

	if (registers.vrd.num != -1) encoding |= (registers.vrd.num << 0);
	if (registers.vrs.num != -1) encoding |= (registers.vrs.num << 8);
	if (registers.vrt.num != -1) encoding |= (registers.vrt.num << 16);
	if (vfpuSize != -1)
	{
		if (vfpuSize & 1) encoding |= (1 << 7);
		if (vfpuSize & 2) encoding |= (1 << 15);
	}
	
	g_fileManager->write(&encoding,4);
}

void CMipsInstruction::Encode()
{
	if (subInstruction != NULL)
		subInstruction->Encode();

	if (Opcode.flags & MO_VFPU)
		encodeVfpu();
	else
		encodeNormal();
}

void CMipsInstruction::writeTempData(TempData& tempData)
{
	char str[256];

	if (subInstruction != NULL)
		subInstruction->writeTempData(tempData);

	int pos = sprintf(str,"   ");
	pos += formatOpcodeName(&str[pos]);
	while (pos < 11) str[pos++] = ' ';
	str[pos] = 0;
	formatParameters(&str[pos]);

	tempData.writeLine(RamPos,convertUtf8ToWString(str));
}
