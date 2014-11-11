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

static const char* vpfxstRegisters = "xyzw";
static const char* vpfxstConstants[8] = {"0","1","2","1/2","3","1/3","1/4","1/6"};

bool parseVpfxsParameter(const char* text, int& result, int& RetLen)
{
	const char* start = text;
	result = 0;

	if (*text++ != '[')
		return false;

	for (int i = 0; i < 4; i++)
	{
		char buffer[64];
		
		if (*text == 0 || *text == ']')
			return false;

		// extract element from text, so we don't have to worry about whitespace
		int pos = 0;
		while (*text != ',' && *text != 0 &&  *text != ']')
		{
			if (*text == ' ' || *text == '\t')
			{
				text++;
				continue;
			}

			buffer[pos++] = *text++;
		}

		if (*text == ',')
			text++;

		if (pos == 0)
			return false;

		buffer[pos] = 0;
		pos = 0;
		
		// negation
		if (buffer[pos] == '-')
		{
			result |= 1 << (16+i);
			pos++;
		}

		// abs
		bool abs = false;
		if (buffer[pos] == '|')
		{
			result |= 1 << (8+i);
			abs = true;
			pos++;
		}

		// check for register
		const char* reg;
		if ((reg = strchr(vpfxstRegisters,buffer[pos])) != NULL)
		{
			result |= (reg-vpfxstRegisters) << (i*2);

			if (abs && buffer[pos+1] != '|')
				return false;

			continue;
		}

		// abs is invalid with constants
		if (abs)
			return false;

		result |= 1 << (12+i);

		int constNum = -1;
		for (int k = 0; k < 8; k++)
		{
			if (strcmp(&buffer[pos],vpfxstConstants[k]) == 0)
			{
				constNum = k;
				break;
			}
		}

		if (constNum == -1)
			return false;

		result |= (constNum & 3) << (i*2);
		if (constNum & 4)
			result |= 1 << (8+i);
	}
	
	if (*text++ != ']')
		return false;

	RetLen = text-start;
	return true;
}

bool parseVpfxdParameter(const char* text, int& result, int& RetLen)
{
	const char* start = text;
	result = 0;

	if (*text++ != '[')
		return false;

	for (int i = 0; i < 4; i++)
	{
		char buffer[64];
		
		if (*text == 0 || *text == ']')
		{
			if (i == 3)
				break;
			return false;
		}

		// extract element from text, so we don't have to worry about whitespace
		int pos = 0;
		while (*text != ',' && *text != 0 &&  *text != ']')
		{
			if (*text == ' ' || *text == '\t')
			{
				text++;
				continue;
			}

			buffer[pos++] = *text++;
		}

		if (*text == ',')
			text++;

		if (pos == 0)
			continue;

		buffer[pos] = 0;
		int length = pos;
		pos = 0;
		
		if (length > 0 && buffer[length-1] == 'm')
		{
			buffer[--length] = 0;
			result |= 1 << (8+i);
		}

		if (strcmp(buffer,"0-1") == 0 || strcmp(buffer,"0:1") == 0)
			result |= 1 << (2*i);
		else if (strcmp(buffer,"-1-1") == 0 || strcmp(buffer,"-1:1") == 0)
			result |= 3 << (2*i);
	}
	
	if (*text++ != ']')
		return false;

	RetLen = text-start;
	return true;
}


// http://code.google.com/p/jpcsp/source/browse/trunk/src/jpcsp/Allegrex/VfpuState.java?spec=svn3676&r=3383#1196
static int floatToHalfFloat(int i)
{
	int s = ((i >> 16) & 0x00008000); // sign
	int e = ((i >> 23) & 0x000000ff) - (127 - 15); // exponent
	int f = ((i >> 0) & 0x007fffff); // fraction

	// need to handle NaNs and Inf?
	if (e <= 0) {
		if (e < -10) {
			if (s != 0) {
				// handle -0.0
				return 0x8000;
			}
			return 0;
		}
		f = (f | 0x00800000) >> (1 - e);
		return s | (f >> 13);
	} else if (e == 0xff - (127 - 15)) {
		if (f == 0) {
			// Inf
			return s | 0x7c00;
		}
		// NAN
		f >>= 13;
		f = 0x3ff; // PSP always encodes NaN with this value
		return s | 0x7c00 | f | ((f == 0) ? 1 : 0);
	}

	if (e > 30) {
		// Overflow
		return s | 0x7c00;
	}

	return s | (e << 10) | (f >> 13);
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

	const char* SourceEncoding = SourceOpcode.name;
	while (*SourceEncoding != 0)
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

bool CMipsInstruction::LoadEncoding(const tMipsOpcode& SourceOpcode, const char* Line)
{
	int RetLen;
	CStringList List;
	bool Immediate = false;
	
	immediateType = MipsImmediateType::None;
	extInsType = MipsExtInsSizeType::None;
	registers.reset();
	vectorCondition = -1;

	if (vfpuSize == -1)
	{
		if (SourceOpcode.flags & MO_VFPU_SINGLE)
			vfpuSize = 0;
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
				if (MipsCheckImmediate(Line,extInsImmediate.expression,RetLen) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				
				if (*SourceEncoding == 'e')				
					extInsType = MipsExtInsSizeType::Ext;
				else if (*SourceEncoding == 'i')
					extInsType = MipsExtInsSizeType::Ins;
				else
					return false;
				
				SourceEncoding++;
				break;
			case 'r':	// forced register
				if (MipsGetRegister(Line,RetLen) != *(SourceEncoding+1)) return false;
				Line += RetLen;
				SourceEncoding += 2;
				break;
			case 'C':
				if ((vectorCondition = MipsGetVectorCondition(Line, RetLen)) == -1) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'W':	// vpfxst argument
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
	
	if (extInsImmediate.expression.isLoaded())
	{
		if (extInsImmediate.expression.check() == false)
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
	return dest-start;
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

	if (extInsType != MipsExtInsSizeType::None)
	{
		if (extInsImmediate.expression.evaluate(extInsImmediate.value,true) == false)
			return false;

		extInsImmediate.originalValue = extInsImmediate.value;

		if (extInsImmediate.value > 32 || extInsImmediate.value == 0)
		{
			Logger::queueError(Logger::Error,L"Immediate value %02X out of range",extInsImmediate.value);
			return false;
		}
		
		extInsImmediate.value--;
		if (extInsType == MipsExtInsSizeType::Ins)
			extInsImmediate.value += immediate.value;
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

	if (extInsType != MipsExtInsSizeType::None)
		encoding |= extInsImmediate.value << 11;

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
