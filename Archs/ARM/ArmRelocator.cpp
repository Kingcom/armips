#include "stdafx.h"
#include "ArmRelocator.h"


inline int signExtend(int value, int bitsLength)
{
	return (value << (32-bitsLength)) >> (32-bitsLength);
}

int getThumbCallAddend(unsigned int opcode)
{
	unsigned short first = opcode & 0xFFFF;
	unsigned short second = (opcode >> 16) & 0xFFFF;

	int a = ((first & 0x7FF) << 11) | (second & 0x7FF);
	return signExtend(a << 1,23);
}

unsigned int updateThumbCallOpcode(unsigned int oldOpcode, int newAddend)
{
	unsigned short first = (oldOpcode & 0xFFFF) & ~0x7FF;
	unsigned short second = ((oldOpcode >> 16) & 0xFFFF) & ~0x7FF;

	newAddend >>= 1;
	first |= (newAddend >> 11) & 0x7FF;
	second |= (newAddend & 0x7FF);

	return first | (second << 16);
}


/*
	S = symbol address
	T = 1 if symbol is a thumb mode function, 0 otherwise
	P = offset of opcode
	A = addend
*/

bool ArmElfRelocator::relocateOpcode(int type, RelocationData& data)
{
	int t = (data.targetSymbolType == STT_FUNC && data.targetSymbolInfo != 0) ? 1 : 0;
	int p = data.opcodeOffset;
	int s = data.relocationBase;

	switch (type)
	{
	case R_ARM_ABS32:		// (S + A) | T
		data.opcode = (data.opcode + data.relocationBase) | t;
		break;
	case R_ARM_THM_CALL:	// ((S + A) | T) – P
		{
			int a = getThumbCallAddend(data.opcode);
			int value = ((s+a) | t) - p;
			data.opcode = updateThumbCallOpcode(data.opcode,value);
		}
		break;
	case R_ARM_CALL:		// ((S + A) | T) – P
	case R_ARM_JUMP24:		// ((S + A) | T) – P
		{
			int condField = (data.opcode >> 8) & 0xF;
			int opField = (data.opcode & 0x7FFFFF) << 2;
			int a = signExtend(opField,25);
			int value = ((s+a) | t) - p;

			data.opcode &= ~0x7FFFFF;
			data.opcode |= (value >> 2) & 0x7FFFFF;
		}
		break;
	default:
		return false;
	}

	return true;
}

void ArmElfRelocator::setSymbolAddress(RelocationData& data, unsigned int symbolAddress, int symbolType)
{
	if (symbolType == STT_FUNC)
	{
		data.targetSymbolInfo = symbolAddress & 1;
		symbolAddress &= ~1;
	}

	data.symbolAddress = symbolAddress;
	data.targetSymbolType = symbolType;
}
