#include "stdafx.h"
#include "ArmRelocator.h"
#include "Util/Util.h"
#include "Arm.h"
#include "Core/Common.h"

inline int signExtend(int value, int bitsLength)
{
	return (value << (32-bitsLength)) >> (32-bitsLength);
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
	int p = (int) data.opcodeOffset;
	int s = (int) data.relocationBase;

	switch (type)
	{
	case R_ARM_ABS32:		// (S + A) | T
	case R_ARM_TARGET1:
		data.opcode = (int) (data.opcode + data.relocationBase) | t;
		break;
	case R_ARM_THM_CALL:	// ((S + A) | T) – P
		{
			unsigned short first = data.opcode & 0xFFFF;
			unsigned short second = (data.opcode >> 16) & 0xFFFF;
			int opField = ((first & 0x7FF) << 11) | (second & 0x7FF);
			int a = signExtend(opField << 1,23);
			int value = (s+a) - p;

			first &= ~0x7FF;
			second &= ~0x7FF;

			if (t == 1)
			{
				if (data.relocationBase % 2)
				{
					data.errorMessage = L"Branch target must be halfword aligned";
					return false;
				}
			} else {
				if (arm9 == false)
				{
					data.errorMessage = L"Cannot call ARM function from THUMB code without stub";
					return false;
				}

				if (data.relocationBase % 4)
				{
					data.errorMessage = L"Branch target must be word aligned";
					return false;
				}
				
				second = 0xE800;
			}

			if (abs(value) >= 0x400000)
			{
				data.errorMessage = formatString(L"Branch target %08X out of range",data.relocationBase);
				return false;
			}

			value >>= 1;
			first |= (value >> 11) & 0x7FF;
			second |= value & 0x7FF;
			data.opcode = first | (second << 16);
		}
		break;
	case R_ARM_CALL:		// ((S + A) | T) – P
	case R_ARM_JUMP24:		// ((S + A) | T) – P
		{
			int condField = (data.opcode >> 28) & 0xF;
			int opField = (data.opcode & 0xFFFFFF) << 2;
			data.opcode &= ~0xFFFFFF;

			int a = signExtend(opField,26);
			int value = (s+a) - p;

			if (t == 1)
			{
				if (data.relocationBase % 2)
				{
					data.errorMessage = L"Branch target must be halfword aligned";
					return false;
				}

				if (type == R_ARM_JUMP24)
				{
					data.errorMessage = L"Cannot jump from ARM to THUMB without link";
					return false;
				}

				if (arm9 == false)
				{
					data.errorMessage = L"Cannot call THUMB function from ARM code without stub";
					return false;
				}

				if (condField != 0xE)
				{
					data.errorMessage = L"Cannot convert conditional bl into blx";
					return false;
				}

				data.opcode = 0xFA000000;
				if (value & 2)
					data.opcode |= (1 << 24);
			} else {
				if (data.relocationBase % 4)
				{
					data.errorMessage = L"Branch target must be word aligned";
					return false;
				}
			}
			
			if (abs(value) >= 0x2000000)
			{
				data.errorMessage = formatString(L"Branch target %08X out of range",data.relocationBase);
				return false;
			}

			data.opcode |= (value >> 2) & 0xFFFFFF;
		}
		break;
	default:
		data.errorMessage = formatString(L"Unknown ARM relocation type %d",type);
		return false;
	}

	return true;
}

void ArmElfRelocator::setSymbolAddress(RelocationData& data, u64 symbolAddress, int symbolType)
{
	if (symbolType == STT_FUNC)
	{
		data.targetSymbolInfo = symbolAddress & 1;
		symbolAddress &= ~1;
	}

	data.symbolAddress = symbolAddress;
	data.targetSymbolType = symbolType;
}

void ArmElfRelocator::writeCtorStub(std::vector<ElfRelocatorCtor>& ctors)
{
	if (ctors.size() == 0)
	{
		Arm.AssembleOpcode(L"bx",L"r14");
		return;
	}

	// arm7 can't blx to a register. a stub needs to be added
	bool thumbStub = false;
	std::wstring thumbStubName;

	if (arm9 == false && Arm.GetThumbMode())
	{
		thumbStub = true;
		thumbStubName = Global.symbolTable.getUniqueLabelName();
	}

	// initialization
	Arm.AssembleOpcode(L"push",L"r4-r7,r14");

	std::wstring tableLabel = Global.symbolTable.getUniqueLabelName();
	Arm.AssembleOpcode(L"ldr",formatString(L"r4,=%s",tableLabel));
	Arm.AssembleOpcode(L"ldr",formatString(L"r5,=%s+0x%08X",tableLabel,ctors.size()*8));
	
	// actual function
	std::wstring loopStartLabel = Global.symbolTable.getUniqueLabelName();
	addAssemblerLabel(loopStartLabel);
	Arm.AssembleOpcode(L"ldr",L"r6,[r4]");
	Arm.AssembleOpcode(L"ldr",L"r7,[r4,4]");
	Arm.AssembleOpcode(L"add",L"r4,8");

	std::wstring innerLoopLabel = Global.symbolTable.getUniqueLabelName();
	addAssemblerLabel(innerLoopLabel);
	
	Arm.AssembleOpcode(L"ldr",L"r0,[r6]");
	Arm.AssembleOpcode(L"add",L"r6,4");

	if (thumbStub)
		Arm.AssembleOpcode(L"bl",thumbStubName);
	else
		Arm.AssembleOpcode(L"blx",L"r0");

	// finish inner loop
	Arm.AssembleOpcode(L"cmp",L"r6,r7");
	Arm.AssembleOpcode(L"blt",innerLoopLabel);

	// finish outer loop
	Arm.AssembleOpcode(L"cmp",L"r4,r5");
	Arm.AssembleOpcode(L"blt",loopStartLabel);

	// finish function
	if (thumbStub)
	{
		Arm.AssembleOpcode(L"pop",L"r4-r7");
		Arm.AssembleOpcode(L"pop",L"r0");

		addAssemblerLabel(thumbStubName);
		Arm.AssembleOpcode(L"bx",L"r0");
	} else {
		Arm.AssembleOpcode(L"pop",L"r4-r7,r15");
	}

	// add data
	Arm.AssembleDirective(L".pool",L"");
	Arm.AssembleDirective(L".align",L"4");
	addAssemblerLabel(tableLabel);

	std::wstring data;
	for (size_t i = 0; i < ctors.size(); i++)
	{
		data += ctors[i].symbolName;
		data += formatString(L",%s+0x%08X,",ctors[i].symbolName,ctors[i].size);
	}

	data.pop_back();	// remove trailing comma
	Arm.AssembleDirective(L".word",data);
}
