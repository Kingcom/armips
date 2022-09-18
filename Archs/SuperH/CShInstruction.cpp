#include "Archs/SuperH/CShInstruction.h"

#include "Archs/SuperH/SuperH.h"
#include "Archs/SuperH/ShOpcodes.h"
#include "Archs/SuperH/ShParser.h"
#include "Core/Common.h"
#include "Core/FileManager.h"
#include "Core/Misc.h"

CShInstruction::CShInstruction(ShOpcodeData& opcode, ShImmediateData& immediate, ShRegisterData& registers)
{
	this->opcodeData = opcode;
	this->immediateData = immediate;
	this->registerData = registers;
}

CShInstruction::~CShInstruction()
{

}

int getImmediateBits(ShImmediateType type)
{
	switch (type)
	{
	case ShImmediateType::Immediate4:
		return 4;
	case ShImmediateType::Immediate8:
		return 8;
	case ShImmediateType::Immediate12:
		return 12;
	default:
		return 0;
	}
}

bool CShInstruction::Validate(const ValidateState &state)
{
	bool Result = false;

	//bool previousNop = addNop;
	//addNop = false;

	RamPos = g_fileManager->getVirtualAddress();
	if (RamPos % 2)
	{
		Logger::queueError(Logger::Error, "opcode not aligned to word boundary");
		return false;
	}

	// check immediates
	if (immediateData.primary.type != ShImmediateType::None)
	{
		if (immediateData.primary.expression.isLoaded())
		{
			if (!immediateData.primary.expression.evaluateInteger(immediateData.primary.value))
			{
				Logger::queueError(Logger::Error, "Invalid immediate expression");
				return false;
			}

			immediateData.primary.originalValue = immediateData.primary.value;
		}

		if (opcodeData.opcode.flags & SH_MUSTBEALIGNED)	// immediate must be aligned
		{
			if ((opcodeData.opcode.flags & SH_IMM16) && (immediateData.primary.value % 2))
			{
				Logger::queueError(Logger::Error, "Immediate must be 2-byte aligned");
				return false;
			}

			if ((opcodeData.opcode.flags & SH_IMM32) && (immediateData.primary.value % 4))
			{
				Logger::queueError(Logger::Error, "Immediate must be 4-byte aligned");
				return false;
			}
		}

		int immediateBits = getImmediateBits(immediateData.primary.type);
		int maxImmediate = ((1 << immediateBits) - 1);

		if (opcodeData.opcode.flags & SH_IMMREL)	// relative
		{
			int ldSize = (opcodeData.opcode.flags & SH_IMM32) ? 4 : 2;
			int rPos = (opcodeData.opcode.flags & SH_IMM32) ? (RamPos & 0xFFFFFFFFFFFFFFFC) : RamPos;
			int range = maxImmediate*ldSize;
			int hiRange = (opcodeData.opcode.flags & SH_IMMSIGNED) ? (range/2) : range;
			int lowRange = (opcodeData.opcode.flags & SH_IMMSIGNED) ? -(range/2) : 0;

			int num = (int) (immediateData.primary.value-rPos-4);

			if (num > hiRange || num < lowRange)
			{
				Logger::queueError(Logger::Error, "Branch/move target %08X out of range", immediateData.primary.value);
				return false;
			}
			immediateData.primary.value = num;
		}

		if (opcodeData.opcode.flags & SH_IMM16)
			immediateData.primary.value = immediateData.primary.value >> 1;
		else if (opcodeData.opcode.flags & SH_IMM32)
			immediateData.primary.value = immediateData.primary.value >> 2;
		
		unsigned int mask = (0xFFFFFFFF << (32-immediateBits)) >> (32-immediateBits);
		int digits = (immediateBits+3) / 4;

		if ((unsigned int)std::abs(immediateData.primary.value) > mask)
		{
			Logger::queueError(Logger::Error, "Immediate value 0x%0*X out of range",digits,immediateData.primary.value);
			return false;
		}

		immediateData.primary.value &= mask;
	}

#if 0
	// check load delay
	if (Sh.hasLoadDelay() && Sh.GetLoadDelay() && !IgnoreLoadDelay)
	{
		bool fix = false;

		if (registerData.grd.num != -1 && registerData.grd.num == Sh.GetLoadDelayRegister())
		{
			Logger::queueError(Logger::Warning, "register %S may not be available due to load delay",registerData.grd.name);
			fix = true;
		} else if (registerData.grs.num != -1 && registerData.grs.num == Sh.GetLoadDelayRegister())
		{
			Logger::queueError(Logger::Warning, "register %S may not be available due to load delay",registerData.grs.name);
			fix = true;
		} else if (registerData.grt.num != -1 && registerData.grt.num == Sh.GetLoadDelayRegister()
			&& !(opcodeData.opcode.flags & MO_IGNORERTD))
		{
			Logger::queueError(Logger::Warning, "register %S may not be available due to load delay",registerData.grt.name);
			fix = true;
		}

		if (Sh.GetFixLoadDelay() && fix)
		{
			addNop = true;
			Logger::queueError(Logger::Notice, "added nop to ensure correct behavior");
		}
	}

	if ((opcodeData.opcode.flags & MO_NODELAYSLOT) && Sh.GetDelaySlot() && !IgnoreLoadDelay)
	{
		Logger::queueError(Logger::Error, "This instruction can't be in a delay slot");
	}

	Sh.SetDelaySlot((opcodeData.opcode.flags & MO_DELAY) != 0);

	// now check if this opcode causes a load delay
	if (Sh.hasLoadDelay())
		Sh.SetLoadDelay((opcodeData.opcode.flags & MO_DELAYRT) != 0,registerData.grt.num);
	
	if (previousNop != addNop)
		Result = true;
#endif

	g_fileManager->advanceMemory(2); //addNop ? 4 : 2
	return Result;
}

void CShInstruction::Encode() const
{
	//if (addNop)
	//	g_fileManager->writeU32(0);

	uint16_t encoding = opcodeData.opcode.base;

	switch (opcodeData.opcode.format)
	{
	case SHFMT_0:     // xxxx xxxx xxxx xxxx
		break;
	case SHFMT_N:     // xxxx nnnn xxxx xxxx
		encoding |= (registerData.grt.num & 0xF) << 8;
		break;
	case SHFMT_M:     // xxxx mmmm xxxx xxxx
		encoding |= (registerData.grs.num & 0xF) << 8;
		break;
	case SHFMT_NM:    // xxxx nnnn mmmm xxxx
		encoding |= (registerData.grt.num & 0xF) << 8;
		encoding |= (registerData.grs.num & 0xF) << 4;
		break;
	case SHFMT_MD:    // xxxx xxxx mmmm dddd
		encoding |= (registerData.grs.num & 0xF) << 4;
		encoding |= immediateData.primary.value;
		break;
	case SHFMT_ND4:   // xxxx xxxx nnnn dddd
		encoding |= (registerData.grt.num & 0xF) << 4;
		encoding |= immediateData.primary.value;
		break;
	case SHFMT_NMD:   // xxxx nnnn mmmm dddd
		encoding |= (registerData.grt.num & 0xF) << 8;
		encoding |= (registerData.grs.num & 0xF) << 4;
		encoding |= immediateData.primary.value;
		break;
	case SHFMT_D:     // xxxx xxxx dddd dddd
	case SHFMT_D12:   // xxxx dddd dddd dddd
		encoding |= immediateData.primary.value;
		break;
	case SHFMT_ND8:   // xxxx nnnn dddd dddd
		encoding |= (registerData.grt.num & 0xF) << 8;
		encoding |= immediateData.primary.value;
		break;
	case SHFMT_I:     // xxxx xxxx iiii iiii
		encoding |= immediateData.primary.value;
		break;
	case SHFMT_NI:    // xxxx nnnn iiii iiii
		encoding |= (registerData.grt.num & 0xF) << 8;
		encoding |= immediateData.primary.value;
		break;
	}

	g_fileManager->writeU16((uint16_t)encoding);
}

void CShInstruction::writeTempData(TempData& tempData) const
{
	ShOpcodeFormatter formatter;
	tempData.writeLine(RamPos, formatter.formatOpcode(opcodeData,registerData,immediateData));
}
