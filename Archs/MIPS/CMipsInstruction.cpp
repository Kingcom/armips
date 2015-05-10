#include "stdafx.h"
#include "CMipsInstruction.h"
#include "Core/Common.h"
#include "Mips.h"
#include "MipsOpcodes.h"
#include "Core/FileManager.h"

CMipsInstruction::CMipsInstruction(MipsOpcodeData& opcode, MipsImmediateData& immediate, MipsRegisterData& registers)
{
	this->opcodeData = opcode;
	this->immediateData = immediate;
	this->registerData = registers;

	addNop = false;
	IgnoreLoadDelay = Mips.GetIgnoreDelay();
}

CMipsInstruction::~CMipsInstruction()
{

}

int CMipsInstruction::formatOpcodeName(char* dest) const
{
/*	const char* encoding = Opcode.name;
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
	return (int) (dest-start);*/
	return 0;
}

void CMipsInstruction::formatParameters(char* dest) const
{
/*	const char* encoding = Opcode.encoding;

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
	*dest = 0;*/
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

// http://code.google.com/p/jpcsp/source/browse/trunk/src/jpcsp/Allegrex/VfpuState.java?spec=svn3676&r=3383#1196
int CMipsInstruction::floatToHalfFloat(int i)
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
		return s | 0x7fff;
	}

	if (e > 30) {
		// Overflow
		return s | 0x7c00;
	}

	return s | (e << 10) | (f >> 13);
}

bool CMipsInstruction::Validate()
{
	bool Result = false;

	bool previousNop = addNop;
	addNop = false;

	RamPos = g_fileManager->getVirtualAddress();
	if (RamPos % 4)
	{
		Logger::queueError(Logger::Error,L"opcode not aligned to word boundary");
		return false;
	}

	// check immediates
	if (immediateData.primary.type != MipsImmediateType::None)
	{
		if (immediateData.primary.expression.isLoaded())
		{
			if (immediateData.primary.expression.evaluateInteger(immediateData.primary.value) == false)
				return false;

			immediateData.primary.originalValue = immediateData.primary.value;
		}

		if (immediateData.primary.type == MipsImmediateType::ImmediateHalfFloat)
			immediateData.primary.value = floatToHalfFloat(immediateData.primary.originalValue);

		if (opcodeData.opcode.flags & MO_IMMALIGNED)	// immediate must be aligned
		{
			if (immediateData.primary.value % 4)
			{
				Logger::queueError(Logger::Error,L"Immediate must be word aligned");
				return false;
			}
		}

		if (opcodeData.opcode.flags & MO_IPCA)	// absolute value >> 2)
		{
			immediateData.primary.value = (immediateData.primary.value >> 2) & 0x3FFFFFF;
		} else if (opcodeData.opcode.flags & MO_IPCR)	// relative 16 bit value
		{
			int num = (int) (immediateData.primary.value-RamPos-4);
			
			if (num > 0x20000 || num < (-0x20000))
			{
				Logger::queueError(Logger::Error,L"Branch target %08X out of range",immediateData.primary.value);
				return false;
			}
			immediateData.primary.value = num >> 2;
		}
		
		int immediateBits = getImmediateBits(immediateData.primary.type);
		unsigned int mask = (0xFFFFFFFF << (32-immediateBits)) >> (32-immediateBits);
		int digits = (immediateBits+3) / 4;

		if ((unsigned int)std::abs(immediateData.primary.value) > mask)
		{
			Logger::queueError(Logger::Error,L"Immediate value %0*X out of range",digits,immediateData.primary.value);
			return false;
		}

		immediateData.primary.value &= mask;
	}

	if (immediateData.secondary.type != MipsImmediateType::None)
	{
		if (immediateData.secondary.expression.isLoaded())
		{
			if (immediateData.secondary.expression.evaluateInteger(immediateData.secondary.value) == false)
				return false;

			immediateData.secondary.originalValue = immediateData.secondary.value;
		}

		switch (immediateData.secondary.type)
		{
		case MipsImmediateType::Ext:
		case MipsImmediateType::Ins:
			if (immediateData.secondary.value > 32 || immediateData.secondary.value == 0)
			{
				Logger::queueError(Logger::Error,L"Immediate value %02X out of range",immediateData.secondary.value);
				return false;
			}
		
			immediateData.secondary.value--;
			if (immediateData.secondary.type == MipsImmediateType::Ins)
				immediateData.secondary.value += immediateData.primary.value;
			break;
		}
	}

	// check load delay
	if (Mips.hasLoadDelay() && Mips.GetLoadDelay() && IgnoreLoadDelay == false && !(opcodeData.opcode.flags & MO_IGNORERTD))
	{
		bool fix = false;

		if (registerData.grd.num != -1 && registerData.grd.num == Mips.GetLoadDelayRegister())
		{
			Logger::queueError(Logger::Warning,L"register %S may not be available due to load delay",registerData.grd.name);
			fix = true;
		} else if (registerData.grs.num != -1 && registerData.grs.num == Mips.GetLoadDelayRegister())
		{
			Logger::queueError(Logger::Warning,L"register %S may not be available due to load delay",registerData.grs.name);
			fix = true;
		} else if (registerData.grt.num != -1 && registerData.grt.num == Mips.GetLoadDelayRegister())
		{
			Logger::queueError(Logger::Warning,L"register %S may not be available due to load delay",registerData.grt.name);
			fix = true;
		}

		if (Mips.GetFixLoadDelay() == true && fix == true)
		{
			addNop = true;
			Logger::queueError(Logger::Notice,L"added nop to ensure correct behavior");
		}
	}

	if ((opcodeData.opcode.flags & MO_NODELAYSLOT) && Mips.GetDelaySlot() == true && IgnoreLoadDelay == false)
	{
		Logger::queueError(Logger::Error,L"This instruction can't be in a delay slot");
	}

	Mips.SetDelaySlot(opcodeData.opcode.flags & MO_DELAY ? true : false);

	// now check if this opcode causes a load delay
	if (Mips.hasLoadDelay())
		Mips.SetLoadDelay(opcodeData.opcode.flags & MO_DELAYRT ? true : false,registerData.grt.num);
	
	if (previousNop != addNop)
		Result = true;

	g_fileManager->advanceMemory(addNop ? 8 : 4);
	return Result;
}

void CMipsInstruction::encodeNormal() const
{
	int encoding = opcodeData.opcode.destencoding;

	if (registerData.grs.num != -1) encoding |= MIPS_RS(registerData.grs.num);	// source reg
	if (registerData.grt.num != -1) encoding |= MIPS_RT(registerData.grt.num);	// target reg
	if (registerData.grd.num != -1) encoding |= MIPS_RD(registerData.grd.num);	// dest reg
	
	if (registerData.frt.num != -1) encoding |= MIPS_FT(registerData.frt.num);	// float target reg
	if (registerData.frs.num != -1) encoding |= MIPS_FS(registerData.frs.num);	// float source reg
	if (registerData.frd.num != -1) encoding |= MIPS_FD(registerData.frd.num);	// float dest reg
	
	if (registerData.ps2vrt.num != -1) encoding |= (registerData.ps2vrt.num << 16);	// ps2 vector target reg
	if (registerData.ps2vrs.num != -1) encoding |= (registerData.ps2vrs.num << 21);	// ps2 vector source reg
	if (registerData.ps2vrd.num != -1) encoding |= (registerData.ps2vrd.num << 6);	// ps2 vector dest reg

	if (!(opcodeData.opcode.flags & MO_VFPU_MIXED) && registerData.vrt.num != -1)			// vfpu rt
		encoding |= registerData.vrt.num << 16;

	switch (immediateData.primary.type)
	{
	case MipsImmediateType::Immediate5:
	case MipsImmediateType::Immediate20:
		encoding |= immediateData.primary.value << 6;
		break;
	case MipsImmediateType::Immediate16:
	case MipsImmediateType::Immediate26:
	case MipsImmediateType::Immediate20_0:
	case MipsImmediateType::ImmediateHalfFloat:
		encoding |= immediateData.primary.value;
		break;
	case MipsImmediateType::Immediate8:
		encoding |= immediateData.primary.value << 16;
		break;
	}

	switch (immediateData.secondary.type)
	{
	case MipsImmediateType::Ext:
	case MipsImmediateType::Ins:
		encoding |= immediateData.secondary.value << 11;
		break;
	case MipsImmediateType::Cop2BranchType:
		encoding |= immediateData.secondary.value << 18;
		break;
	}

	if (opcodeData.opcode.flags & MO_VFPU_MIXED)
	{
		// always vrt
		encoding |= registerData.vrt.num >> 5;
		encoding |= (registerData.vrt.num & 0x1F) << 16;
	}
	
	g_fileManager->write(&encoding,4);
}

void CMipsInstruction::encodeVfpu() const
{
	int encoding = opcodeData.opcode.destencoding;
	
	if (opcodeData.vectorCondition != -1) encoding |= (opcodeData.vectorCondition << 0);
	if (registerData.vrd.num != -1) encoding |= (registerData.vrd.num << 0);
	if (registerData.vrs.num != -1) encoding |= (registerData.vrs.num << 8);
	if (registerData.vrt.num != -1) encoding |= (registerData.vrt.num << 16);
	if (opcodeData.vfpuSize != -1 && (opcodeData.opcode.flags & (MO_VFPU_PAIR|MO_VFPU_SINGLE|MO_VFPU_TRIPLE|MO_VFPU_QUAD)) == 0)
	{
		if (opcodeData.vfpuSize & 1) encoding |= (1 << 7);
		if (opcodeData.vfpuSize & 2) encoding |= (1 << 15);
	}

	if (registerData.grt.num != -1) encoding |= (registerData.grt.num << 16);
	
	switch (immediateData.primary.type)
	{
	case MipsImmediateType::Immediate5:
		encoding |= immediateData.primary.value << 16;
		break;
	case MipsImmediateType::Immediate7:
		encoding |= immediateData.primary.value << 0;
		break;
	}

	g_fileManager->write(&encoding,4);
}

void CMipsInstruction::Encode() const
{
	if (addNop)
	{
		u32 zero = 0;
		g_fileManager->write(&zero,4);
	}

	if (opcodeData.opcode.flags & MO_VFPU)
		encodeVfpu();
	else
		encodeNormal();
}

void CMipsInstruction::writeTempData(TempData& tempData) const
{
/*	char str[256];


	int pos = sprintf(str,"   ");
	pos += formatOpcodeName(&str[pos]);
	while (pos < 11) str[pos++] = ' ';
	str[pos] = 0;
	formatParameters(&str[pos]);

	tempData.writeLine(RamPos,convertUtf8ToWString(str));*/
}
