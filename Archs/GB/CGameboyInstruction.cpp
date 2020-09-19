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
}

bool CGameboyInstruction::Validate(const ValidateState& state)
{
	if (Opcode.flags & GB_LOAD_REG8_REG8)
	{
		if (Vars.LeftParam.num == GB_REG8_MEMHL && Vars.RightParam.num == GB_REG8_MEMHL)
		{
			Logger::queueError(Logger::Error, L"ld (hl),(hl) not allowed");
			return false;
		}
	}
	if (Opcode.flags & (GB_IMMEDIATE_S8 | GB_IMMEDIATE_U8 | GB_IMMEDIATE_U16))
	{
		ExpressionValue value = Vars.ImmediateExpression.evaluate();
		if (value.isValid() && value.isInt())
		{
			Vars.Immediate = value.intValue;
		}

		int64_t min = 0;
		int64_t max = 0;
		if (Opcode.flags & GB_IMMEDIATE_U8)
		{
			min = 0;
			max = 255;
		}
		else if (Opcode.flags & GB_IMMEDIATE_S8)
		{
			min = -128;
			max = 127;
		}
		else if (Opcode.flags & GB_IMMEDIATE_U16)
		{
			min = 0;
			max = 65535;
		}

		if (Vars.Immediate < min || Vars.Immediate > max)
		{
			Logger::queueError(Logger::Error, L"Immediate %i out of range", Vars.Immediate);
			return false;
		}
	}

	g_fileManager->advanceMemory(Opcode.length);

	return false;
}

void CGameboyInstruction::Encode() const
{
	unsigned char encoding = Opcode.encoding;

	if (Opcode.flags & GB_PREFIX)
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


	if (Opcode.flags & GB_IMMEDIATE_U16)
	{
		g_fileManager->writeU16((uint16_t)(Vars.Immediate & 0xFFFF));
	}
	else if (Opcode.flags & (GB_IMMEDIATE_U8 | GB_IMMEDIATE_S8))
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
