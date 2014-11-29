#include "stdafx.h"
#include "CMipsInstruction.h"
#include "Core/Common.h"
#include "Mips.h"
#include "MipsOpcodes.h"
#include "MipsPSP.h"
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

bool CMipsInstruction::Load(const char* Name, const char* Params)
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


bool CMipsInstruction::parseOpcode(const tMipsOpcode& SourceOpcode, const char* Line)
{
	vfpuSize = -1;
	hasFixedSecondaryImmediate = false;

	const char* SourceEncoding = SourceOpcode.name;
	while (*SourceEncoding != 0)
	{
		if (*Line == 0) return false;

		int RetLen;
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
		case 'B':	// cop2 branch condition
			if (parseCop2BranchCondition(Line,secondaryImmediate.originalValue,RetLen) == false) return false;
			secondaryImmediateType = MipsSecondaryImmediateType::Cop2BranchType;
			secondaryImmediate.value = secondaryImmediate.originalValue;
			hasFixedSecondaryImmediate = true;

			SourceEncoding++;
			Line += RetLen;
			break;
		default:
			if (*SourceEncoding++ != *Line++) return false;
			break;
		}
	}
	
	if (*Line != 0)	return false;	// there's something else, bad
	return true;
}

bool CMipsInstruction::LoadEncoding(const tMipsOpcode& SourceOpcode, const char* Line)
{
	int RetLen;
	CStringList List;
	bool Immediate = false;
	
	immediateType = MipsImmediateType::None;
	if (!hasFixedSecondaryImmediate)
		secondaryImmediateType = MipsSecondaryImmediateType::None;
	registers.reset();
	vectorCondition = -1;

	if (vfpuSize == -1)
	{
		if (SourceOpcode.flags & MO_VFPU_SINGLE)
			vfpuSize = 0;
		else if (SourceOpcode.flags & MO_VFPU_PAIR)
			vfpuSize = 1;
		else if (SourceOpcode.flags & MO_VFPU_TRIPLE)
			vfpuSize = 2;
		else if (SourceOpcode.flags & MO_VFPU_QUAD)
			vfpuSize = 3;
	}

	const char* SourceEncoding = SourceOpcode.encoding;
	const char* OriginalLine = Line;

	while (*Line == ' ' || *Line == '\t') Line++;

	if (!(*SourceEncoding == 0 && *Line == 0))
	{
		while (*SourceEncoding != 0)
		{
			while (*Line == ' ' || *Line == '\t') Line++;
			if (*Line == 0) return false;

			int actualSize;
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
			case 'v':	// psp vfpu vector register
				actualSize = vfpuSize;
				if (*(SourceEncoding+1) == 'S')
				{
					SourceEncoding++;
					actualSize = 0;	// single
				}

				switch (*(SourceEncoding+1))
				{
				case 's':
					if (parseVFPURegister(Line,registers.vrs,actualSize) == false) return false;
					if (registers.vrs.type != MIPSVFPU_VECTOR) return false;
					if ((SourceOpcode.flags & MO_VFPU_6BIT) && (registers.vrs.num & 0x40)) return false;
					Line += 4;
					break;
				case 't':
					if (parseVFPURegister(Line,registers.vrt,actualSize) == false) return false;
					if (registers.vrt.type != MIPSVFPU_VECTOR) return false;
					if ((SourceOpcode.flags & MO_VFPU_6BIT) && (registers.vrt.num & 0x40)) return false;
					Line += 4;
					break;
				case 'd':
					if (parseVFPURegister(Line,registers.vrd,actualSize) == false) return false;
					if (registers.vrd.type != MIPSVFPU_VECTOR) return false;
					if ((SourceOpcode.flags & MO_VFPU_6BIT) && (registers.vrd.num & 0x40)) return false;
					Line += 4;
					break;
				case 'c':
					if (parseVfpuControlRegister(Line,registers.vrd,RetLen) == false) return false;
					Line += RetLen;
					break;
				default:
					return false;
				}

				SourceEncoding += 2;
				break;
			case 'm':	// psp vfpu amtrix register
				switch (*(SourceEncoding+1))
				{
				case 's':
					if (parseVFPURegister(Line,registers.vrs,vfpuSize) == false) return false;
					if (registers.vrs.type != MIPSVFPU_MATRIX) return false;
					if (SourceOpcode.flags & MO_TRANSPOSE_VS) registers.vrs.num ^= 0x20;
					Line += 4;
					break;
				case 't':
					if (parseVFPURegister(Line,registers.vrt,vfpuSize) == false) return false;
					if (registers.vrt.type != MIPSVFPU_MATRIX) return false;
					Line += 4;
					break;
				case 'd':
					if (parseVFPURegister(Line,registers.vrd,vfpuSize) == false) return false;
					if (registers.vrd.type != MIPSVFPU_MATRIX) return false;
					Line += 4;
					break;
				default:
					return false;
				}

				SourceEncoding += 2;
				break;
			case 'V':	// ps2 vector registers
				switch (*(SourceEncoding+1))
				{
				case 's':
					if (MipsGetPs2VectorRegister(Line,RetLen,registers.ps2vrs) == false) return false;
					Line += RetLen;
					break;
				case 't':
					if (MipsGetPs2VectorRegister(Line,RetLen,registers.ps2vrt) == false) return false;
					Line += RetLen;
					break;
				case 'd':
					if (MipsGetPs2VectorRegister(Line,RetLen,registers.ps2vrd) == false) return false;
					Line += RetLen;
					break;
				default:
					return false;
				}
				SourceEncoding += 2;
				break;
			case 'i':	// standard immediate
				if (MipsCheckImmediate(Line,immediate.expression,RetLen) == false) return false;
				Line += RetLen;
				SourceEncoding++;

				if (*SourceEncoding == 'h')	// half float
				{
					SourceEncoding++;
					immediateType = MipsImmediateType::ImmediateHalfFloat;
				} else {
					int num = 0;
					while (*SourceEncoding >= '0' && *SourceEncoding <= '9')
					{
						num = num*10 + *SourceEncoding-'0';
						SourceEncoding++;
					}

					switch (num)
					{
					case 5:
						immediateType = MipsImmediateType::Immediate5;
						break;
					case 7:
						immediateType = MipsImmediateType::Immediate7;
						break;
					case 8:
						immediateType = MipsImmediateType::Immediate8;
						break;
					case 16:
						immediateType = MipsImmediateType::Immediate16;
						break;
					case 20:
						immediateType = MipsImmediateType::Immediate20;
						break;
					case 26:
						immediateType = MipsImmediateType::Immediate26;
						break;
					default:
						return false;
					}
				}
				break;
			case 'j':
				switch (*(SourceEncoding+1))
				{
				case 'e':
					if (MipsCheckImmediate(Line,secondaryImmediate.expression,RetLen) == false) return false;
					secondaryImmediateType = MipsSecondaryImmediateType::Ext;
					break;
				case 'i':
					if (MipsCheckImmediate(Line,secondaryImmediate.expression,RetLen) == false) return false;
					secondaryImmediateType = MipsSecondaryImmediateType::Ins;
					break;
				case 'b':
					if (parseCop2BranchCondition(Line,secondaryImmediate.originalValue,RetLen) == false) return false;
					secondaryImmediateType = MipsSecondaryImmediateType::Cop2BranchType;
					secondaryImmediate.value = secondaryImmediate.originalValue;
					break;
				}

				Line += RetLen;
				SourceEncoding += 2;
				break;
			case 'r':	// forced register
				if (MipsGetRegister(Line,RetLen) != *(SourceEncoding+1)) return false;
				Line += RetLen;
				SourceEncoding += 2;
				break;
			case 'C':
				if ((vectorCondition = parseVFPUCondition(Line, RetLen)) == -1) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'W':	// vfpu argument
				switch (*(SourceEncoding+1))
				{
				case 's':
					if (parseVpfxsParameter(Line,immediate.originalValue,RetLen) == false) return false;
					immediateType = MipsImmediateType::Immediate20_0;
					break;
				case 'd':
					if (parseVpfxdParameter(Line,immediate.originalValue,RetLen) == false) return false;
					immediateType = MipsImmediateType::Immediate16;
					break;
				case 'c':
					if (parseVcstParameter(Line,immediate.originalValue,RetLen) == false) return false;
					immediateType = MipsImmediateType::Immediate5;
					break;
				case 'r':
					if (parseVfpuVrot(Line,immediate.originalValue,vfpuSize,RetLen) == false) return false;
					immediateType = MipsImmediateType::Immediate5;
					break;
				default:
					return false;
				}

				immediate.value = immediate.originalValue;
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
	
	if (secondaryImmediate.expression.isLoaded())
	{
		if (secondaryImmediate.expression.check() == false)
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
	const char* encoding = Opcode.name;
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
	return (int) (dest-start);
}

void CMipsInstruction::formatParameters(char* dest)
{
	const char* encoding = Opcode.encoding;

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
		case 'V':
			switch (*(encoding+1))
			{
			case 'd':
				dest += sprintf(dest,"%s",registers.ps2vrd.name);
				break;
			case 's':
				dest += sprintf(dest,"%s",registers.ps2vrs.name);
				break;
			case 't':
				dest += sprintf(dest,"%s",registers.ps2vrt.name);
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
	case MipsImmediateType::Immediate5:
		return 5;
	case MipsImmediateType::Immediate7:
		return 7;
	case MipsImmediateType::Immediate8:
		return 8;
	case MipsImmediateType::Immediate16:
	case MipsImmediateType::ImmediateHalfFloat:
		return 16;
	case MipsImmediateType::Immediate20:
	case MipsImmediateType::Immediate20_0:
		return 20;
	case MipsImmediateType::Immediate26:
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
	if (immediateType != MipsImmediateType::None)
	{
		if (immediate.expression.isLoaded())
		{
			if (immediate.expression.evaluate(immediate.value,true) == false)
				return false;

			immediate.originalValue = immediate.value;
		}

		if (immediateType == MipsImmediateType::ImmediateHalfFloat)
			immediate.value = floatToHalfFloat(immediate.originalValue);

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
			int num = (int) (immediate.value-RamPos-4);
			
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

	if (secondaryImmediateType != MipsSecondaryImmediateType::None)
	{
		if (secondaryImmediate.expression.isLoaded())
		{
			if (secondaryImmediate.expression.evaluate(secondaryImmediate.value,true) == false)
				return false;

			secondaryImmediate.originalValue = secondaryImmediate.value;
		}

		switch (secondaryImmediateType)
		{
		case MipsSecondaryImmediateType::Ext:
		case MipsSecondaryImmediateType::Ins:
			if (secondaryImmediate.value > 32 || secondaryImmediate.value == 0)
			{
				Logger::queueError(Logger::Error,L"Immediate value %02X out of range",secondaryImmediate.value);
				return false;
			}
		
			secondaryImmediate.value--;
			if (secondaryImmediateType == MipsSecondaryImmediateType::Ins)
				secondaryImmediate.value += immediate.value;
			break;
		}
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

	if (registers.grs.num != -1) encoding |= MIPS_RS(registers.grs.num);	// source reg
	if (registers.grt.num != -1) encoding |= MIPS_RT(registers.grt.num);	// target reg
	if (registers.grd.num != -1) encoding |= MIPS_RD(registers.grd.num);	// dest reg
	
	if (registers.frt.num != -1) encoding |= MIPS_FT(registers.frt.num);	// float target reg
	if (registers.frs.num != -1) encoding |= MIPS_FS(registers.frs.num);	// float source reg
	if (registers.frd.num != -1) encoding |= MIPS_FD(registers.frd.num);	// float dest reg
	
	if (registers.ps2vrt.num != -1) encoding |= (registers.ps2vrt.num << 16);	// ps2 vector target reg
	if (registers.ps2vrs.num != -1) encoding |= (registers.ps2vrs.num << 21);	// ps2 vector source reg
	if (registers.ps2vrd.num != -1) encoding |= (registers.ps2vrd.num << 6);	// ps2 vector dest reg

	if (!(Opcode.flags & MO_VFPU_MIXED) && registers.vrt.num != -1)			// vfpu rt
		encoding |= registers.vrt.num << 16;

	switch (immediateType)
	{
	case MipsImmediateType::Immediate5:
	case MipsImmediateType::Immediate20:
		encoding |= immediate.value << 6;
		break;
	case MipsImmediateType::Immediate16:
	case MipsImmediateType::Immediate26:
	case MipsImmediateType::Immediate20_0:
	case MipsImmediateType::ImmediateHalfFloat:
		encoding |= immediate.value;
		break;
	case MipsImmediateType::Immediate8:
		encoding |= immediate.value << 16;
		break;
	}

	switch (secondaryImmediateType)
	{
	case MipsSecondaryImmediateType::Ext:
	case MipsSecondaryImmediateType::Ins:
		encoding |= secondaryImmediate.value << 11;
		break;
	case MipsSecondaryImmediateType::Cop2BranchType:
		encoding |= secondaryImmediate.value << 18;
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
	
	if (vectorCondition != -1) encoding |= (vectorCondition << 0);
	if (registers.vrd.num != -1) encoding |= (registers.vrd.num << 0);
	if (registers.vrs.num != -1) encoding |= (registers.vrs.num << 8);
	if (registers.vrt.num != -1) encoding |= (registers.vrt.num << 16);
	if (vfpuSize != -1 && (Opcode.flags & (MO_VFPU_PAIR|MO_VFPU_SINGLE|MO_VFPU_TRIPLE|MO_VFPU_QUAD)) == 0)
	{
		if (vfpuSize & 1) encoding |= (1 << 7);
		if (vfpuSize & 2) encoding |= (1 << 15);
	}

	if (registers.grt.num != -1) encoding |= (registers.grt.num << 16);
	
	switch (immediateType)
	{
	case MipsImmediateType::Immediate5:
		encoding |= immediate.value << 16;
		break;
	case MipsImmediateType::Immediate7:
		encoding |= immediate.value << 0;
		break;
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
