#include "stdafx.h"
#include "CZ80Instruction.h"
#include "z80Opcodes.h"
#include "Core/Common.h"

CZ80Instruction::CZ80Instruction()
{
	NoCheckError = false;
	Loaded = false;
	OpcodeSize = 0;
}

bool CZ80Instruction::LoadEncoding(const tZ80Opcode& SourceOpcode, char* Line)
{
	char ImmediateBuffer[512];

	int RetLen;
	CStringList List;
	bool Immediate = false;

	const char* SourceEncoding = SourceOpcode.mask;
	char* OriginalLine = Line;

	while (*Line == ' ' || *Line == '\t') Line++;

	if (!(*SourceEncoding == 0 && *Line == 0))
	{
		while (*SourceEncoding != 0)
		{
			while (*Line == ' ' || *Line == '\t') Line++;

			switch (*SourceEncoding)
			{
			case 'I':	// immediate
				if (z80CheckImmediate(Line,ImmediateBuffer,RetLen,List) == false) return false;
				Vars.ImmediateBitLen = *(SourceEncoding+1);
				Line += RetLen;
				SourceEncoding += 2;
				break;
			case 'M':	// 16 bit register
				if (z80GetRegister16(Line,RetLen,Vars.Reg16) == false) return false;
				if (((1 << Vars.Reg16.Number) & *(SourceEncoding+1)) == 0) return false;
				Line += RetLen;
				SourceEncoding += 2;
				break;
			case 'N':	// 8 bit register
				if (z80GetRegister8(Line,RetLen,Vars.Reg8) == false) return false;
				if (((1 << Vars.Reg8.Number) & *(SourceEncoding+1)) == 0) return false;
				Line += RetLen;
				SourceEncoding += 2;
				break;
			case 'Q':	// fixed 8 bit register
				if (z80GetRegister8(Line,RetLen) != *(SourceEncoding+1)) return false;
				Line += RetLen;
				SourceEncoding += 2;
				break;
			case 'R':	// fixed 16 bit register
				if (z80GetRegister16(Line,RetLen) != *(SourceEncoding+1)) return false;
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

	if (Opcode.flags & Z80_IMMEDIATE)
	{
		if (CheckPostfix(List,true) == false)
		{
			Logger::printError(Logger::Error,L"Invalid expression \"%S\"",ImmediateBuffer);
			NoCheckError = true;
			return false;
		}
		Vars.ImmediateExpression.Load(List);
	}

	OpcodeSize = Opcode.TotalLen;
	return true;
}

bool CZ80Instruction::Load(char *Name, char *Params)
{
	bool paramfail = false;
	NoCheckError = false;

	for (int z = 0; Z80Opcodes[z].name != NULL; z++)
	{
		if (strcmp(Name,Z80Opcodes[z].name) == 0)
		{
			if (LoadEncoding(Z80Opcodes[z],Params) == true)
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
			Logger::printError(Logger::Error,L"Parameter failure \"%S\"",Params);
		} else {
			Logger::printError(Logger::Error,L"Invalid opcode \"%S\"",Name);
		}
	}
	return false;
}

void CZ80Instruction::Encode()
{
	unsigned char WriteBuffer[4];
	int WritePos = 0;

	for (int i = 0; i < Opcode.OpcodeLen; i++)
	{
		WriteBuffer[WritePos++] = Opcode.Opcodes[i];
	}

	if (Opcode.flags & Z80_IMMEDIATE)
	{
		if (Opcode.flags & Z80_ADD_IMMEDIATE_TIMES_8)	// last byte += immediate*8
		{
			WriteBuffer[WritePos-1] += Vars.Immediate*8;
		} else {
			for (int i = 0; i < Vars.ImmediateBitLen; i += 8)
			{
				WriteBuffer[WritePos++] = Vars.Immediate >> (i*8);
			}
		}
	}

	if (Opcode.flags & Z80_8BIT_REGISTER_NIBBLE)
	{
		WriteBuffer[WritePos-1] = (WriteBuffer[WritePos-1] & 0xF0) | Vars.Reg8.Number;
	}

	if (Opcode.flags & Z80_8BIT_REGISTER_TIMES_8)
	{
		WriteBuffer[WritePos-1] += Vars.Reg8.Number*8;
	}

	if (Opcode.flags & Z80_16BIT_REGISTER_TIMES_16)
	{
		WriteBuffer[WritePos-1] += Vars.Reg16.Number*16;
	}

}
