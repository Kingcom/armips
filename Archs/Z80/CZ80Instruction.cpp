#include "Archs/Z80/CZ80Instruction.h"
#include "Archs/Z80/Z80Opcodes.h"
#include "Core/Common.h"
#include "Core/Expression.h"
#include "Core/FileManager.h"
#include "Core/Misc.h"
#include "Util/Util.h"

CZ80Instruction::CZ80Instruction(const tZ80Opcode& sourceOpcode, Z80OpcodeVariables& vars)
{
	this->Opcode = sourceOpcode;
	this->Vars = vars;
	this->RamPos = 0;
}

bool CZ80Instruction::Validate(const ValidateState& state)
{
	RamPos = g_fileManager->getVirtualAddress();

	Vars.Length = Opcode.length;
	Vars.Encoding = Opcode.encoding;
	Vars.WriteImmediate8 = false;
	Vars.WriteImmediate16 = false;

	// Evaluate immediate
	if (Opcode.flags & Z80_HAS_IMMEDIATE)
	{
		if (!Vars.ImmediateExpression.evaluateInteger(Vars.Immediate))
		{
			Logger::queueError(Logger::Error, L"Invalid expression");
			return false;
		}
		if ((Opcode.flags & Z80_HAS_2_IMMEDIATES) && !Vars.ImmediateExpression2.evaluateInteger(Vars.Immediate2))
		{
			Logger::queueError(Logger::Error, L"Invalid expression");
			return false;
		}
		if (Vars.IsNegative)
		{
			Vars.Immediate = -Vars.Immediate;
		}
		if (Opcode.flags & Z80_JUMP_RELATIVE)
		{
			Vars.Immediate = (Vars.Immediate - RamPos - 2);
		}

		int64_t min = INT64_MIN;
		int64_t max = INT64_MAX;
		int64_t min2 = INT64_MIN;
		int64_t max2 = INT64_MAX;
		Vars.WriteImmediate8 = false;
		Vars.WriteImmediate16 = false;
		if (Opcode.flags & Z80_INTERRUPT_MODE)
		{
			min = 0;
			max = 2;
		}
		else if (Opcode.flags & Z80_IMMEDIATE_U3)
		{
			min = 0;
			max = 7;
		}
		else if (Opcode.flags & Z80_IMMEDIATE_U8)
		{
			min = 0;
			max = 255;
			Vars.WriteImmediate8 = true;
		}
		else if (Opcode.flags & Z80_IMMEDIATE_S8)
		{
			min = -128;
			max = 127;
			Vars.WriteImmediate8 = true;
		}
		else if (Opcode.flags & Z80_IMMEDIATE_U16)
		{
			min = 0;
			max = 65535;
			Vars.WriteImmediate16 = true;
		}
		if (Opcode.flags & Z80_IMMEDIATE2_U8)
		{
			min2 = 0;
			max2 = 255;
		}

		// add <-> sub
		if ((Opcode.flags & Z80_ADD_SUB_IMMEDIATE) && Vars.Immediate < 0)
		{
			// Change opcode
			Vars.Encoding ^= 0x10;
			Vars.Immediate = -Vars.Immediate;
		}
		if (Opcode.flags & Z80_NEGATE_IMM)
		{
			Vars.Immediate = -Vars.Immediate;
		}

		if (Z80.GetVersion() == Z80ArchType::Gameboy)
		{
			// Special loads in range 0xFF00 - 0xFFFF
			if (Vars.RightParam.num == Z80_REG8_A && Vars.Immediate >= 0xFF00 && !(Opcode.flags & Z80_IMMEDIATE_U3))
			{
				// ld (0xFF00+u8),a can be encoded as E0 XX instead
				Vars.Encoding = 0xE0;
				Vars.Length = 2;
				Vars.Immediate &= 0xFF;
				Vars.RightParam.num = 0;
				Vars.WriteImmediate8 = true;
				Vars.WriteImmediate16 = false;
			}
			else if (Vars.LeftParam.num == Z80_REG8_A && Vars.Immediate >= 0xFF00)
			{
				// ld a,(0xFF00+u8) can be encoded as F0 XX instead
				Vars.Encoding = 0xF0;
				Vars.Length = 2;
				Vars.Immediate &= 0xFF;
				Vars.LeftParam.num = 0;
				Vars.WriteImmediate8 = true;
				Vars.WriteImmediate16 = false;
			}
		}

		if (Vars.Immediate < min || Vars.Immediate > max)
		{
			if (Opcode.flags & Z80_JUMP_RELATIVE)
			{
				Logger::queueError(Logger::Error, L"Jump target %04X out of range", Vars.Immediate);
			}
			else if (Opcode.flags & Z80_INTERRUPT_MODE)
			{
				Logger::queueError(Logger::Error, L"Interrupt mode %i out of range", Vars.Immediate);
			}
			else
			{
				Logger::queueError(Logger::Error, L"Immediate %i out of range", Vars.Immediate);
			}
			return false;
		}
		if ((Opcode.flags & Z80_HAS_2_IMMEDIATES) && (Vars.Immediate2 < min2 || Vars.Immediate2 > max2))
		{
			Logger::queueError(Logger::Error, L"Immediate %i out of range", Vars.Immediate2);
			return false;
		}

		if (Opcode.flags & Z80_RST)
		{
			bool invalidRST = false;
			if (Z80.GetVersion() == Z80ArchType::Gameboy && Vars.Immediate >= 0x00 && Vars.Immediate <= 0x7)
			{
				// Nintendo syntax
				// 1 -> 8, 2 -> 16, 3 -> 24, etc
				Vars.Immediate <<= 3;
			}
			else if (Z80.GetVersion() == Z80ArchType::Ereader && Vars.Immediate != 0x00 && Vars.Immediate != 0x08)
			{
				invalidRST = true;
			}
			else if (Vars.Immediate != 0x00 && Vars.Immediate != 0x08 && Vars.Immediate != 0x10 && Vars.Immediate != 0x18 &&
				Vars.Immediate != 0x20 && Vars.Immediate != 0x28 && Vars.Immediate != 0x30 && Vars.Immediate != 0x38)
			{
				invalidRST = true;
			}

			if (invalidRST)
			{
				Logger::queueError(Logger::Error, L"Invalid RST target %i", Vars.Immediate);
				return false;
			}
		}

		// Move immediate to lhs
		if (Opcode.flags & (Z80_IMMEDIATE_U3 | Z80_RST))
		{
			Vars.LeftParam.name = L"imm";
			Vars.LeftParam.num = Vars.Immediate;
		}
		else if (Opcode.flags & Z80_INTERRUPT_MODE)
		{
			Vars.LeftParam.name = L"imm";
			Vars.LeftParam.num = Vars.Immediate + (Vars.Immediate != 0 ? 1 : 0);
		}
	}

	g_fileManager->advanceMemory(Vars.Length);

	return false;
}

void CZ80Instruction::Encode() const
{
	unsigned char encoding = Vars.Encoding;
	int prefixes = 0;

	if (Z80_IS_PARAM_IX_IY(Opcode.lhs) && Vars.LeftParam.num == Z80_REG16_IX ||
		Z80_IS_PARAM_IX_IY(Opcode.rhs) && Vars.RightParam.num == Z80_REG16_IX)
	{
		g_fileManager->writeU8(0xDD);
		prefixes++;
	}
	else if (Z80_IS_PARAM_IX_IY(Opcode.lhs) && Vars.LeftParam.num == Z80_REG16_IY ||
		Z80_IS_PARAM_IX_IY(Opcode.rhs) && Vars.RightParam.num == Z80_REG16_IY)
	{
		g_fileManager->writeU8(0xFD);
		prefixes++;
	}

	if (Opcode.flags & Z80_PREFIX_CB)
	{
		g_fileManager->writeU8(0xCB);
		prefixes++;
	}
	else if (Opcode.flags & Z80_PREFIX_ED)
	{
		g_fileManager->writeU8(0xED);
		prefixes++;
	}

	if (Opcode.lhs && Opcode.lhsShift >= 0)
	{
		encoding |= (Vars.LeftParam.num & 0x7F) << Opcode.lhsShift;
	}
	if (Opcode.rhs && Opcode.rhsShift >= 0)
	{
		encoding |= (Vars.RightParam.num & 0x7F) << Opcode.rhsShift;
	}

	// If there are 2 prefixes, opcode follows immediate
	if (prefixes < 2)
	{
		g_fileManager->writeU8(encoding);
	}

	// Write immediates
	if (Vars.WriteImmediate16)
	{
		g_fileManager->writeU16((uint16_t)(Vars.Immediate & 0xFFFF));
	}
	else if (Vars.WriteImmediate8)
	{
		g_fileManager->writeU8((uint8_t)(Vars.Immediate & 0xFF));
	}
	if (Opcode.flags & Z80_IMMEDIATE2_U8)
	{
		g_fileManager->writeU8((uint8_t)(Vars.Immediate2 & 0xFF));
	}

	// If there are 2 prefixes, opcode follows immediate
	if (prefixes >= 2)
	{
		g_fileManager->writeU8(encoding);
	}

	if (Opcode.flags & Z80_STOP)
	{
		g_fileManager->writeU8(0x00);
	}
}

void CZ80Instruction::writeTempData(TempData& tempData) const
{
	char str[256];

	int pos = sprintf(str, "   %S", Opcode.name);
	while (pos < 11) str[pos++] = ' ';
	str[pos] = 0;

	tempData.writeLine(RamPos, convertUtf8ToWString(str));
}
