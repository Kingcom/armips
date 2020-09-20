#include "Archs/GB/CGameboyInstruction.h"
#include "Archs/GB/GameboyOpcodes.h"
#include "Core/Common.h"
#include "Core/Expression.h"
#include "Core/FileManager.h"
#include "Core/Misc.h"

CGameboyInstruction::CGameboyInstruction(const tGameboyOpcode& sourceOpcode, GameboyOpcodeVariables& vars)
{
	this->Opcode = sourceOpcode;
	this->Vars = vars;
	this->RamPos = 0;
}

bool CGameboyInstruction::Validate(const ValidateState& state)
{
	RamPos = g_fileManager->getVirtualAddress();

	Vars.Length = Opcode.length;
	Vars.Encoding = Opcode.encoding;
	Vars.WritePrefix = Opcode.flags & GB_PREFIX;
	Vars.WriteImmediate8 = false;
	Vars.WriteImmediate16 = false;

	// ld (hl),(hl) equivalent to halt
	if (Opcode.flags & GB_LOAD_REG8_REG8)
	{
		if (Vars.LeftParam.num == GB_REG8_MEMHL && Vars.RightParam.num == GB_REG8_MEMHL)
		{
			Logger::queueError(Logger::Error, L"ld (hl),(hl) not allowed");
			return false;
		}
	}

	// Evaluate immediate
	if (Opcode.flags & GB_HAS_IMMEDIATE)
	{
		if (!Vars.ImmediateExpression.evaluateInteger(Vars.Immediate))
		{
			Logger::queueError(Logger::Error, L"Invalid expression");
			return false;
		}
		if (Vars.IsNegative)
		{
			Vars.Immediate = -Vars.Immediate;
		}
		if (Opcode.flags & GB_JUMP_RELATIVE)
		{
			Vars.Immediate = (Vars.Immediate - RamPos - 2);
		}

		int64_t min = INT64_MIN;
		int64_t max = INT64_MAX;
		if (Opcode.flags & GB_IMMEDIATE_U3)
		{
			min = 0;
			max = 8;
			Vars.WriteImmediate8 = false;
			Vars.WriteImmediate16 = false;
		}
		if (Opcode.flags & GB_IMMEDIATE_U8)
		{
			min = 0;
			max = 255;
			Vars.WriteImmediate8 = true;
			Vars.WriteImmediate16 = false;
		}
		else if (Opcode.flags & GB_IMMEDIATE_S8)
		{
			min = -128;
			max = 127;
			Vars.WriteImmediate8 = true;
			Vars.WriteImmediate16 = false;
		}
		else if (Opcode.flags & GB_IMMEDIATE_U16)
		{
			min = 0;
			max = 65535;
			Vars.WriteImmediate8 = false;
			Vars.WriteImmediate16 = true;
		}

		// add <-> sub
		if ((Opcode.flags & GB_ADD_SUB_IMMEDIATE) && Vars.Immediate < 0)
		{
			// Change opcode
			Vars.Encoding ^= 0x10;
			Vars.Immediate = -Vars.Immediate;
		}
		if (Opcode.flags & GB_NEGATE_IMM)
		{
			Vars.Immediate = -Vars.Immediate;
		}

		// Special loads in range 0xFF00 - 0xFFFF
		if (Vars.RightParam.num == GB_REG8_A && Vars.Immediate >= 0xFF00)
		{
			// ld (0xFF00+u8),a can be encoded as E0 XX instead
			Vars.Encoding = 0xE0;
			Vars.Length = 2;
			Vars.Immediate &= 0xFF;
			Vars.RightParam.num = 0;
			Vars.WriteImmediate8 = true;
			Vars.WriteImmediate16 = false;
		}
		else if (Vars.LeftParam.num == GB_REG8_A && Vars.Immediate >= 0xFF00)
		{
			// ld a,(0xFF00+u8) can be encoded as F0 XX instead
			Vars.Encoding = 0xF0;
			Vars.Length = 2;
			Vars.Immediate &= 0xFF;
			Vars.LeftParam.num = 0;
			Vars.WriteImmediate8 = true;
			Vars.WriteImmediate16 = false;
		}

		if (Vars.Immediate < min || Vars.Immediate > max)
		{
			if (Opcode.flags & GB_JUMP_RELATIVE)
			{
				Logger::queueError(Logger::Error, L"Jump target %04X out of range", Vars.Immediate);
			}
			else
			{
				Logger::queueError(Logger::Error, L"Immediate %i out of range", Vars.Immediate);
			}
			return false;
		}
		if (Opcode.flags & GB_RST)
		{
			if (Vars.Immediate != 0x00 && Vars.Immediate != 0x08 && Vars.Immediate != 0x10 && Vars.Immediate != 0x18 &&
				Vars.Immediate != 0x20 && Vars.Immediate != 0x28 && Vars.Immediate != 0x30 && Vars.Immediate != 0x38)
			{
				Logger::queueError(Logger::Error, L"Invalid RST target %i", Vars.Immediate);
				return false;
			}
		}

		// Move immediate to lhs
		if (Opcode.flags & (GB_IMMEDIATE_U3 | GB_RST))
		{
			Vars.LeftParam.name = L"imm";
			Vars.LeftParam.num = Vars.Immediate;
		}
	}

	g_fileManager->advanceMemory(Vars.Length);

	return false;
}

void CGameboyInstruction::Encode() const
{
	unsigned char encoding = Vars.Encoding;

	if (Vars.WritePrefix)
	{
		g_fileManager->writeU8(0xCB);
	}

	if (Opcode.lhs && Opcode.lhsShift >= 0)
	{
		encoding |= Vars.LeftParam.num << Opcode.lhsShift;
	}
	if (Opcode.rhs && Opcode.rhsShift >= 0)
	{
		encoding |= Vars.RightParam.num << Opcode.rhsShift;
	}

	g_fileManager->writeU8(encoding);


	if (Vars.WriteImmediate16)
	{
		g_fileManager->writeU16((uint16_t)(Vars.Immediate & 0xFFFF));
	}
	else if (Vars.WriteImmediate8)
	{
		g_fileManager->writeU8((uint8_t)(Vars.Immediate & 0xFF));
	}
	else if (Opcode.flags & GB_STOP)
	{
		g_fileManager->writeU8(0x00);
	}
}

void CGameboyInstruction::writeTempData(TempData& tempData) const
{
}
