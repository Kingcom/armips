#include "Archs/ARM/ArmElfRelocator.h"

#include "Archs/ARM/Arm.h"
#include "Core/Common.h"
#include "Parser/Parser.h"
#include "Util/Util.h"

#include <algorithm>

#include <fmt/format.h>

inline int signExtend(int value, int bitsLength)
{
	return (value << (32-bitsLength)) >> (32-bitsLength);
}

bool ArmElfRelocator::isDummyRelocationType(int type) const
{
	// R_ARM_V4BX marks the position of a bx opcode, and does not
	// cause any actual relocations
	return type == R_ARM_V4BX;
}

int ArmElfRelocator::expectedMachine() const
{
	return EM_ARM;
}

/*
	S = symbol address
	T = 1 if symbol is a thumb mode function, 0 otherwise
	P = offset of opcode
	A = addend
*/

bool ArmElfRelocator::relocateOpcode(int type, const RelocationData& data, std::vector<RelocationAction>& actions, std::vector<std::wstring>& errors)
{
	int t = (data.targetSymbolType == STT_FUNC && data.targetSymbolInfo != 0) ? 1 : 0;
	int p = (int) data.opcodeOffset;
	int s = (int) data.relocationBase;

	uint32_t opcode = data.opcode;
	switch (type)
	{
	case R_ARM_ABS32:		// (S + A) | T
	case R_ARM_TARGET1:
		opcode = (int) (opcode + data.relocationBase) | t;
		break;
	case R_ARM_THM_CALL:	// ((S + A) | T) - P
		{
			unsigned short first = opcode & 0xFFFF;
			unsigned short second = (opcode >> 16) & 0xFFFF;
			int opField = ((first & 0x7FF) << 11) | (second & 0x7FF);
			int a = signExtend(opField << 1,23);
			int value = (s+a) - p;

			first &= ~0x7FF;
			second &= ~0x7FF;

			if (t == 1)
			{
				if (data.relocationBase % 2)
				{
					errors.emplace_back(L"Branch target must be halfword aligned");
					return false;
				}
			} else {
				if (arm9 == false)
				{
					errors.emplace_back(L"Cannot call ARM function from THUMB code without stub");
					return false;
				}

				if (data.relocationBase % 4)
				{
					errors.emplace_back(L"Branch target must be word aligned");
					return false;
				}
				
				second = 0xE800;
			}

			if (abs(value) >= 0x400000)
			{
				errors.emplace_back(fmt::format(L"Branch target {:08X} out of range",data.relocationBase));
				return false;
			}

			value >>= 1;
			first |= (value >> 11) & 0x7FF;
			second |= value & 0x7FF;
			opcode = first | (second << 16);
		}
		break;
	case R_ARM_CALL:		// ((S + A) | T) � P
	case R_ARM_JUMP24:		// ((S + A) | T) � P
		{
			int condField = (opcode >> 28) & 0xF;
			int opField = (opcode & 0xFFFFFF) << 2;
			opcode &= ~0xFFFFFF;

			int a = signExtend(opField,26);
			int value = (s+a) - p;

			if (t == 1)
			{
				if (data.relocationBase % 2)
				{
					errors.emplace_back(L"Branch target must be halfword aligned");
					return false;
				}

				if (type == R_ARM_JUMP24)
				{
					errors.emplace_back(L"Cannot jump from ARM to THUMB without link");
					return false;
				}

				if (arm9 == false)
				{
					errors.emplace_back(L"Cannot call THUMB function from ARM code without stub");
					return false;
				}

				if (condField != 0xE)
				{
					errors.emplace_back(L"Cannot convert conditional bl into blx");
					return false;
				}

				opcode = 0xFA000000;
				if (value & 2)
					opcode |= (1 << 24);
			} else {
				if (data.relocationBase % 4)
				{
					errors.emplace_back(L"Branch target must be word aligned");
					return false;
				}
			}
			
			if (abs(value) >= 0x2000000)
			{
				errors.emplace_back(fmt::format(L"Branch target {:08X} out of range",data.relocationBase));
				return false;
			}

			opcode |= (value >> 2) & 0xFFFFFF;
		}
		break;
	default:
		errors.emplace_back(fmt::format(L"Unknown ARM relocation type {}",type));
		return false;
	}

	actions.emplace_back(data.opcodeOffset, opcode);
	return true;
}

void ArmElfRelocator::setSymbolAddress(RelocationData& data, int64_t symbolAddress, int symbolType)
{
	if (symbolType == STT_FUNC)
	{
		data.targetSymbolInfo = symbolAddress & 1;
		symbolAddress &= ~1;
	}

	data.symbolAddress = symbolAddress;
	data.targetSymbolType = symbolType;
}

const wchar_t* ctorTemplate =
	L"push	r4-r7,r14\n"
	L"ldr	r4,=%ctorTable%\n"
	L"ldr	r5,=%ctorTable%+%ctorTableSize%\n"
	L"%outerLoopLabel%:\n"
	L"ldr	r6,[r4]\n"
	L"ldr	r7,[r4,4]\n"
	L"add	r4,8\n"
	L"%innerLoopLabel%:\n"
	L"ldr	r0,[r6]\n"
	L"add	r6,4\n"
	L".if %simpleMode%\n"
	L"	blx	r0\n"
	L".else\n"
	L"	bl	%stubName%\n"
	L".endif\n"
	L"cmp	r6,r7\n"
	L"blt	%innerLooplabel%\n"
	L"cmp	r4,r5\n"
	L"blt	%outerLoopLabel%\n"
	L".if %simpleMode%\n"
	L"	pop	r4-r7,r15\n"
	L".else\n"
	L"	pop	r4-r7\n"
	L"	pop	r0\n"
	L"	%stubName%:\n"
	L"	bx	r0\n"
	L".endif\n"
	L".pool\n"
	L"%ctorTable%:\n"
	L".word %ctorContent%"
;

std::unique_ptr<CAssemblerCommand> ArmElfRelocator::generateCtorStub(std::vector<ElfRelocatorCtor>& ctors)
{
	std::wstring ctorText;

	Parser parser;
	if (ctors.size() != 0)
	{
		bool simpleMode = arm9 == false && Arm.GetThumbMode();

		// create constructor table
		std::wstring table;
		for (size_t i = 0; i < ctors.size(); i++)
		{
			if (i != 0)
				table += ',';
			table += fmt::format(L"{},{}+0x{:08X}",ctors[i].symbolName,ctors[i].symbolName,ctors[i].size);
		}

		return parser.parseTemplate(ctorTemplate,{
			{ L"%ctorTable%",		Global.symbolTable.getUniqueLabelName() },
			{ L"%ctorTableSize%",	fmt::format(L"{}",ctors.size()*8) },
			{ L"%outerLoopLabel%",	Global.symbolTable.getUniqueLabelName() },
			{ L"%innerLoopLabel%",	Global.symbolTable.getUniqueLabelName() },
			{ L"%stubName%",		Global.symbolTable.getUniqueLabelName() },
			{ L"%simpleMode%",		simpleMode ? L"1" : L"0" },
			{ L"%ctorContent%",		table },
		});
	} else {
		return parser.parseTemplate(L"bx r14");
	}
}
