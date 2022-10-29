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

CShInstruction::~CShInstruction() = default;

int CShInstruction::getImmediateBits(ShImmediateType type)
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

//
// NOTE: SuperH *does* have delayed instructions, but
//       I'm not entirely sure how they work exactly,
//       so leaving this functionality, that was taken
//		 from the MIPS parser, removed.
//       Delayed opcodes are currently marked with the
//       SH_DELAYED flag.
//

bool CShInstruction::Validate(const ValidateState &state)
{
	RamPos = g_fileManager->getVirtualAddress();
	if (RamPos % 2)
	{
		Logger::queueError(Logger::Error, "opcode not aligned to word boundary");
		return false;
	}

	bool memoryAdvanced = false;

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
		
		g_fileManager->advanceMemory(2);
		memoryAdvanced = true;

		int opflags = opcodeData.opcode.flags;
		uint64_t immRelAddr = (opflags & SH_IMM32) ? 
							  ((RamPos+4) & 0xFFFFFFFFFFFFFFFC) : 
							   (RamPos+4);
		
		if (opflags & SH_MUSTBEALIGNED)	// immediate must be aligned
		{
			if (opflags & SH_PCRELMANUAL)
				immediateData.primary.value += (int)RamPos;
		
			uint64_t value = immediateData.primary.value;
		
			if ((opflags & SH_IMM16) && (value % 2))
			{
				Logger::queueError(Logger::Error, "Immediate must be 2-byte aligned");
				return false;
			}

			if ((opflags & SH_IMM32) && (value % 4))
			{
				Logger::queueError(Logger::Error, "Immediate must be 4-byte aligned");
				return false;
			}
		}

		int immediateBits = getImmediateBits(immediateData.primary.type);
		int maxImmediate = ((1 << immediateBits) - 1);

		if (opflags & SH_IMMREL || opflags & SH_PCRELMANUAL) // relative
		{
			int range    = maxImmediate * (opflags & SH_IMM32 ? 4 : 2);
			int hiRange  = (opflags & SH_IMMSIGNED) ?  (range/2) : range;
			int lowRange = (opflags & SH_IMMSIGNED) ? -(range/2) : 0;

			int64_t num = (int64_t) (immediateData.primary.value - immRelAddr);

			if (num > hiRange || num < lowRange)
			{
				Logger::queueError(Logger::Error, "Branch/move target %08X out of range", immediateData.primary.value);
				return false;
			}
			immediateData.primary.value = (int)num;
		}

		if (opflags & SH_IMM16)
			immediateData.primary.value = immediateData.primary.value >> 1;
		else if (opflags & SH_IMM32)
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

	if (!memoryAdvanced)
		g_fileManager->advanceMemory(2);

	return false;
}

void CShInstruction::Encode() const
{
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
