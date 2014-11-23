#include "stdafx.h"
#include "Mips.h"
#include "CMipsInstruction.h"
#include "Core/Common.h"
#include "MipsMacros.h"
#include "Core/Directives.h"
#include "Core/FileManager.h"
#include "MipsElfFile.h"
#include "Commands/CDirectiveFile.h"
#include "PsxRelocator.h"

CMipsArchitecture Mips;

const tMipsRegister MipsRegister[] = {
	{ "r0", 0, 2 }, { "zero", 0, 4}, { "$0", 0, 2 }, { "$zero", 0, 5 },
	{ "at", 1, 2 }, { "r1", 1, 2 }, { "$1", 1, 2 }, { "$at", 1, 3 },
	{ "v0", 2, 2 }, { "r2", 2, 2 }, { "$v0", 2, 3 },
	{ "v1", 3, 2 }, { "r3", 3, 2 }, { "$v1", 3, 3 },
	{ "a0", 4, 2 }, { "r4", 4, 2 }, { "$a0", 4, 3 },
	{ "a1", 5, 2 }, { "r5", 5, 2 }, { "$a1", 5, 3 },
	{ "a2", 6, 2 }, { "r6", 6, 2 }, { "$a2", 6, 3 },
	{ "a3", 7, 2 }, { "r7", 7, 2 }, { "$a3", 7, 3 },
	{ "t0", 8, 2 }, { "r8", 8, 2 }, { "$t0", 8, 3 },
	{ "t1", 9, 2 }, { "r9", 9, 2 }, { "$t1", 9, 3 },
	{ "t2", 10, 2 }, { "r10", 10, 3 }, { "$t2", 10, 3 },
	{ "t3", 11, 2 }, { "r11", 11, 3 }, { "$t3", 11, 3 },
	{ "t4", 12, 2 }, { "r12", 12, 3 }, { "$t4", 12, 3 },
	{ "t5", 13, 2 }, { "r13", 13, 3 }, { "$t5", 13, 3 },
	{ "t6", 14, 2 }, { "r14", 14, 3 }, { "$t6", 14, 3 },
	{ "t7", 15, 2 }, { "r15", 15, 3 }, { "$t7", 15, 3 },
	{ "s0", 16, 2 }, { "r16", 16, 3 }, { "$s0", 16, 3 },
	{ "s1", 17, 2 }, { "r17", 17, 3 }, { "$s1", 17, 3 },
	{ "s2", 18, 2 }, { "r18", 18, 3 }, { "$s2", 18, 3 },
	{ "s3", 19, 2 }, { "r19", 19, 3 }, { "$s3", 19, 3 },
	{ "s4", 20, 2 }, { "r20", 20, 3 }, { "$s4", 20, 3 },
	{ "s5", 21, 2 }, { "r21", 21, 3 }, { "$s5", 21, 3 },
	{ "s6", 22, 2 }, { "r22", 22, 3 }, { "$s6", 22, 3 },
	{ "s7", 23, 2 }, { "r23", 23, 3 }, { "$s7", 23, 3 },
	{ "t8", 24, 2 }, { "r24", 24, 3 }, { "$t8", 24, 3 },
	{ "t9", 25, 2 }, { "r25", 25, 3 }, { "$t9", 25, 3 },
	{ "k0", 26, 2 }, { "r26", 26, 3 }, { "$k0", 26, 3 },
	{ "k1", 27, 2 }, { "r27", 27, 3 }, { "$k1", 27, 3 },
	{ "gp", 28, 2 }, { "r28", 28, 3 }, { "$gp", 28, 3 },
	{ "sp", 29, 2 }, { "r29", 29, 3 }, { "$sp", 29, 3 },
	{ "fp", 30, 2 }, { "r30", 30, 3 }, { "$fp", 30, 3 },
	{ "ra", 31, 2 }, { "r31", 31, 3 }, { "$ra", 31, 3 },
	{ NULL, -1, 0}
};


const tMipsRegister MipsFloatRegister[] = {
	{ "f0", 0, 2},		{ "$f0", 0, 3 },
	{ "f1", 1, 2},		{ "$f1", 1, 3 },
	{ "f2", 2, 2},		{ "$f2", 2, 3 },
	{ "f3", 3, 2},		{ "$f3", 3, 3 },
	{ "f4", 4, 2},		{ "$f4", 4, 3 },
	{ "f5", 5, 2},		{ "$f5", 5, 3 },
	{ "f6", 6, 2},		{ "$f6", 6, 3 },
	{ "f7", 7, 2},		{ "$f7", 7, 3 },
	{ "f8", 8, 2},		{ "$f8", 8, 3 },
	{ "f9", 9, 2},		{ "$f9", 9, 3 },
	{ "f10", 10, 3},	{ "$f10", 10, 4 },
	{ "f11", 11, 3},	{ "$f11", 11, 4 },
	{ "f12", 12, 3},	{ "$f12", 12, 4 },
	{ "f13", 13, 3},	{ "$f13", 13, 4 },
	{ "f14", 14, 3},	{ "$f14", 14, 4 },
	{ "f15", 15, 3},	{ "$f15", 15, 4 },
	{ "f16", 16, 3},	{ "$f16", 16, 4 },
	{ "f17", 17, 3},	{ "$f17", 17, 4 },
	{ "f18", 18, 3},	{ "$f18", 18, 4 },
	{ "f19", 19, 3},	{ "$f19", 19, 4 },
	{ "f20", 20, 3},	{ "$f20", 20, 4 },
	{ "f21", 21, 3},	{ "$f21", 21, 4 },
	{ "f22", 22, 3},	{ "$f22", 22, 4 },
	{ "f23", 23, 3},	{ "$f23", 23, 4 },
	{ "f24", 24, 3},	{ "$f24", 24, 4 },
	{ "f25", 25, 3},	{ "$f25", 25, 4 },
	{ "f26", 26, 3},	{ "$f26", 26, 4 },
	{ "f27", 27, 3},	{ "$f27", 27, 4 },
	{ "f28", 28, 3},	{ "$f28", 28, 4 },
	{ "f29", 29, 3},	{ "$f29", 29, 4 },
	{ "f30", 30, 3},	{ "$f30", 30, 4 },
	{ "f31", 31, 3},	{ "$f31", 31, 4 }
};

const tMipsRegister MipsPs2Cop2FpRegister[] = {
	{ "vf0", 0, 3},		{ "$vf0", 0, 4 },
	{ "vf1", 1, 3},		{ "$vf1", 1, 4 },
	{ "vf2", 2, 3},		{ "$vf2", 2, 4 },
	{ "vf3", 3, 3},		{ "$vf3", 3, 4 },
	{ "vf4", 4, 3},		{ "$vf4", 4, 4 },
	{ "vf5", 5, 3},		{ "$vf5", 5, 4 },
	{ "vf6", 6, 3},		{ "$vf6", 6, 4 },
	{ "vf7", 7, 3},		{ "$vf7", 7, 4 },
	{ "vf8", 8, 3},		{ "$vf8", 8, 4 },
	{ "vf9", 9, 3},		{ "$vf9", 9, 4 },
	{ "vf00", 0, 4},	{ "$vf00", 0, 5 },
	{ "vf01", 1, 4},	{ "$vf01", 1, 5 },
	{ "vf02", 2, 4},	{ "$vf02", 2, 5 },
	{ "vf03", 3, 4},	{ "$vf03", 3, 5 },
	{ "vf04", 4, 4},	{ "$vf04", 4, 5 },
	{ "vf05", 5, 4},	{ "$vf05", 5, 5 },
	{ "vf06", 6, 4},	{ "$vf06", 6, 5 },
	{ "vf07", 7, 4},	{ "$vf07", 7, 5 },
	{ "vf08", 8, 4},	{ "$vf08", 8, 5 },
	{ "vf09", 9, 4},	{ "$vf09", 9, 5 },
	{ "vf10", 10, 4},	{ "$vf10", 10, 5 },
	{ "vf11", 11, 4},	{ "$vf11", 11, 5 },
	{ "vf12", 12, 4},	{ "$vf12", 12, 5 },
	{ "vf13", 13, 4},	{ "$vf13", 13, 5 },
	{ "vf14", 14, 4},	{ "$vf14", 14, 5 },
	{ "vf15", 15, 4},	{ "$vf15", 15, 5 },
	{ "vf16", 16, 4},	{ "$vf16", 16, 5 },
	{ "vf17", 17, 4},	{ "$vf17", 17, 5 },
	{ "vf18", 18, 4},	{ "$vf18", 18, 5 },
	{ "vf19", 19, 4},	{ "$vf19", 19, 5 },
	{ "vf20", 20, 4},	{ "$vf20", 20, 5 },
	{ "vf21", 21, 4},	{ "$vf21", 21, 5 },
	{ "vf22", 22, 4},	{ "$vf22", 22, 5 },
	{ "vf23", 23, 4},	{ "$vf23", 23, 5 },
	{ "vf24", 24, 4},	{ "$vf24", 24, 5 },
	{ "vf25", 25, 4},	{ "$vf25", 25, 5 },
	{ "vf26", 26, 4},	{ "$vf26", 26, 5 },
	{ "vf27", 27, 4},	{ "$vf27", 27, 5 },
	{ "vf28", 28, 4},	{ "$vf28", 28, 5 },
	{ "vf29", 29, 4},	{ "$vf29", 29, 5 },
	{ "vf30", 30, 4},	{ "$vf30", 30, 5 },
	{ "vf31", 31, 4},	{ "$vf31", 31, 5 }
};

bool MipsElfRelocator::relocateOpcode(int type, RelocationData& data)
{
	unsigned int p;

	unsigned int op = data.opcode;
	switch (type)
	{
	case R_MIPS_26: //j, jal
		op = (op & 0xFC000000) | (((op&0x03FFFFFF)+(data.relocationBase>>2))&0x03FFFFFF);
		break;
	case R_MIPS_32:
		op += (int) data.relocationBase;
		break;	
	case R_MIPS_HI16:
		p = (op & 0xFFFF) + (int) data.relocationBase;
		op = (op&0xffff0000) | (((p >> 16) + ((p & 0x8000) != 0)) & 0xFFFF);
		break;
	case R_MIPS_LO16:
		op = (op&0xffff0000) | (((op&0xffff)+data.relocationBase)&0xffff);
		break;
	default:
		data.errorMessage = formatString(L"Unknown MIPS relocation type %d",type);
		return false;
	}

	data.opcode = op;
	return true;
}

void MipsElfRelocator::setSymbolAddress(RelocationData& data, u64 symbolAddress, int symbolType)
{
	data.symbolAddress = symbolAddress;
	data.targetSymbolType = symbolType;
}


void MipsElfRelocator::writeCtorStub(std::vector<ElfRelocatorCtor>& ctors)
{
	if (ctors.size() == 0)
	{
		Mips.AssembleOpcode(L"jr",L"ra");
		Mips.AssembleOpcode(L"nop",L"");
		return;
	}

	Mips.AssembleOpcode(L"addiu",L"sp,-32");
	Mips.AssembleOpcode(L"sw",L"ra,0(sp)");
	Mips.AssembleOpcode(L"sw",L"s0,4(sp)");
	Mips.AssembleOpcode(L"sw",L"s1,8(sp)");
	Mips.AssembleOpcode(L"sw",L"s2,12(sp)");
	Mips.AssembleOpcode(L"sw",L"s3,16(sp)");

	std::wstring tableLabel = Global.symbolTable.getUniqueLabelName();

	Mips.AssembleOpcode(L"li",formatString(L"s0,%s",tableLabel));
	Mips.AssembleOpcode(L"li",formatString(L"s1,%s+0x%08X",tableLabel,ctors.size()*8));

	std::wstring outerLoop = Global.symbolTable.getUniqueLabelName();
	addAssemblerLabel(outerLoop);

	Mips.AssembleOpcode(L"lw",L"s2,(s0)");
	Mips.AssembleOpcode(L"lw",L"s3,4(s0)");
	Mips.AssembleOpcode(L"addiu",L"s0,8");

	std::wstring innerLoop = Global.symbolTable.getUniqueLabelName();
	addAssemblerLabel(innerLoop);

	Mips.AssembleOpcode(L"lw",L"a0,(s2)");
	Mips.AssembleOpcode(L"jalr",L"a0");
	Mips.AssembleOpcode(L"addiu",L"s2,4h");

	Mips.AssembleOpcode(L"bne",formatString(L"s2,s3,%s",innerLoop));
	Mips.AssembleOpcode(L"nop",L"");
	
	Mips.AssembleOpcode(L"bne",formatString(L"s0,s1,%s",outerLoop));
	Mips.AssembleOpcode(L"nop",L"");
	
	Mips.AssembleOpcode(L"lw",L"ra,0(sp)");
	Mips.AssembleOpcode(L"lw",L"s0,4(sp)");
	Mips.AssembleOpcode(L"lw",L"s1,8(sp)");
	Mips.AssembleOpcode(L"lw",L"s2,12(sp)");
	Mips.AssembleOpcode(L"lw",L"s3,16(sp)");
	Mips.AssembleOpcode(L"jr",L"ra");
	Mips.AssembleOpcode(L"addiu",L"sp,32");

	// add data
	addAssemblerLabel(tableLabel);

	std::wstring data;
	for (size_t i = 0; i < ctors.size(); i++)
	{
		data += ctors[i].symbolName;
		data += formatString(L",%s+0x%08X,",ctors[i].symbolName,ctors[i].size);
	}

	data.pop_back();	// remove trailing comma
	Mips.AssembleDirective(L".word",data);
}

bool MipsDirectiveResetDelay(ArgumentList& List, int flags)
{
	Mips.SetIgnoreDelay(true);
	return true;
}

bool MipsDirectiveFixLoadDelay(ArgumentList& List, int flags)
{
	Mips.SetFixLoadDelay(true);
	return true;
}

bool MipsDirectiveLoadElf(ArgumentList& list, int flags)
{
	DirectiveLoadMipsElf* command = new DirectiveLoadMipsElf(list);
	AddAssemblerCommand(command);
	return true;
}

bool MipsDirectiveImportObj(ArgumentList& list, int flags)
{
	if (Mips.GetVersion() == MARCH_PSX)
	{
		DirectivePsxObjImport* command = new DirectivePsxObjImport(list);
		AddAssemblerCommand(command);
		return true;
	}
	
	DirectiveObjImport* command = new DirectiveObjImport(list);
	AddAssemblerCommand(command);
	return true;
}

const tDirective MipsDirectives[] = {
	{ L".resetdelay",		0,	0,	&MipsDirectiveResetDelay,	0 },
	{ L".fixloaddelay",		0,	0,	&MipsDirectiveFixLoadDelay,	0 },
	{ L".loadelf",			1,	2,	&MipsDirectiveLoadElf,		0 },
	{ L".importobj",		1,	2,	&MipsDirectiveImportObj,		0 },
	{ L".importlib",		1,	2,	&MipsDirectiveImportObj,		0 },
	{ NULL,					0,	0,	NULL,	0 }
};

bool CMipsArchitecture::AssembleDirective(const std::wstring& name, const std::wstring& args)
{
	if (directiveAssemble(MipsDirectives,name,args) == true)
		return true;
	return directiveAssembleGlobal(name,args);
}

CMipsArchitecture::CMipsArchitecture()
{
	FixLoadDelay = false;
	IgnoreLoadDelay = false;
	LoadDelay = false;
	LoadDelayRegister = 0;
	DelaySlot = false;
	Version = MARCH_INVALID;
}

void CMipsArchitecture::AssembleOpcode(const std::wstring& name, const std::wstring& args)
{
	if (MipsCheckMacro((char*)convertWStringToUtf8(name).c_str(),(char*)convertWStringToUtf8(args).c_str()) == false)
	{
		CMipsInstruction* Opcode = new CMipsInstruction();
		if (Opcode->Load((char*)convertWStringToUtf8(name).c_str(),(char*)convertWStringToUtf8(args).c_str()) == false)
		{
			delete Opcode;
			return;
		}
		AddAssemblerCommand(Opcode);
		g_fileManager->advanceMemory(4);
	}
	SetIgnoreDelay(false);
}

void CMipsArchitecture::NextSection()
{
	LoadDelay = false;
	LoadDelayRegister = 0;
	DelaySlot = false;
}

int CMipsArchitecture::GetWordSize()
{
	switch (Version)
	{
	case MARCH_PSX:
	case MARCH_N64:
	case MARCH_PSP:
		return 4;
	case MARCH_PS2:
		return 8;
	default:
		return 0;
	}
}

IElfRelocator* CMipsArchitecture::getElfRelocator()
{
	switch (Version)
	{
	case MARCH_PS2:
	case MARCH_PSP:
		return new MipsElfRelocator();
	case MARCH_PSX:
	case MARCH_N64:
	default:
		return NULL;
	}
}


void CMipsArchitecture::SetLoadDelay(bool Delay, int Register)
{
	LoadDelay = Delay;
	LoadDelayRegister = Register;
}

bool MipsGetRegister(const char* source, int& RetLen, MipsRegisterInfo& Result)
{
	for (int z = 0; MipsRegister[z].name != NULL; z++)
	{
		int len = MipsRegister[z].len;
		if (strncmp(MipsRegister[z].name,source,len) == 0)	// okay so far
		{
			if (source[len] == ',' || source[len] == '\n'  || source[len] == 0
				|| source[len] == ')'  || source[len] == '(' || source[len] == '-')	// one of these has to come after a register
			{
				memcpy(Result.name,source,len);
				Result.name[len] = 0;
				Result.num = MipsRegister[z].num;
				RetLen = len;
				return true;
			}
		}
	}
	return false;
}

int MipsGetRegister(const char* source, int& RetLen)
{
	for (int z = 0; MipsRegister[z].name != NULL; z++)
	{
		int len = MipsRegister[z].len;
		if (strncmp(MipsRegister[z].name,source,len) == 0)	// okay so far
		{
			if (source[len] == ',' || source[len] == '\n'  || source[len] == 0
				|| source[len] == ')'  || source[len] == '(' || source[len] == '-')	// one of these has to come after a register
			{
				RetLen = len;
				return MipsRegister[z].num;
			}
		}
	}
	return -1;
}


bool MipsGetFloatRegister(const char* source, int& RetLen, MipsRegisterInfo& Result)
{
	for (int z = 0; MipsFloatRegister[z].name != NULL; z++)
	{
		int len = MipsFloatRegister[z].len;
		if (strncmp(MipsFloatRegister[z].name,source,len) == 0)	// okay so far
		{
			if (source[len] == ',' || source[len] == '\n'  || source[len] == 0
				|| source[len] == ')'  || source[len] == '(' || source[len] == '-')	// one of these has to come after a register
			{
				memcpy(Result.name,source,len);
				Result.name[len] = 0;
				Result.num = MipsFloatRegister[z].num;
				RetLen = len;
				return true;
			}
		}
	}
	return false;
}

bool MipsGetPs2VectorRegister(const char* source, int& RetLen, MipsRegisterInfo& Result)
{
	for (int z = 0; MipsPs2Cop2FpRegister[z].name != NULL; z++)
	{
		int len = MipsPs2Cop2FpRegister[z].len;
		if (strncmp(MipsPs2Cop2FpRegister[z].name,source,len) == 0)	// okay so far
		{
			if (source[len] == ',' || source[len] == '\n'  || source[len] == 0
				|| source[len] == ')'  || source[len] == '(' || source[len] == '-')	// one of these has to come after a register
			{
				memcpy(Result.name,source,len);
				Result.name[len] = 0;
				Result.num = MipsPs2Cop2FpRegister[z].num;
				RetLen = len;
				return true;
			}
		}
	}
	return false;
}

int MipsGetFloatRegister(const char* source, int& RetLen)
{
	for (int z = 0; MipsFloatRegister[z].name != NULL; z++)
	{
		int len = MipsFloatRegister[z].len;
		if (strncmp(MipsFloatRegister[z].name,source,len) == 0)	// okay so far
		{
			if (source[len] == ',' || source[len] == '\n'  || source[len] == 0
				|| source[len] == ')'  || source[len] == '(' || source[len] == '-')	// one of these has to come after a register
			{
				RetLen = len;
				return MipsFloatRegister[z].num;
			}
		}
	}
	return -1;
}


bool MipsCheckImmediate(const char* Source, MathExpression& Dest, int& RetLen)
{
	char Buffer[512];
	int BufferPos = 0;
	int l;

	if (MipsGetRegister(Source,l) != -1)	// error
	{
		return false;
	}

	int SourceLen = 0;

	while (true)
	{
		if (*Source == '\'' && *(Source+2) == '\'')
		{
			Buffer[BufferPos++] = *Source++;
			Buffer[BufferPos++] = *Source++;
			Buffer[BufferPos++] = *Source++;
			SourceLen+=3;
			continue;
		}

		if (*Source == 0 || *Source == '\n' || *Source == ',')
		{
			Buffer[BufferPos] = 0;
			break;
		}
		if ( *Source == ' ' || *Source == '\t')
		{
			Source++;
			SourceLen++;
			continue;
		}


		if (*Source == '(')	// could be part of the opcode
		{
			if (MipsGetRegister(Source+1,l) != -1)	// end
			{
				Buffer[BufferPos] = 0;
				break;
			}
		}
		Buffer[BufferPos++] = *Source++;
		SourceLen++;
	}

	if (BufferPos == 0) return false;
	RetLen = SourceLen;

	return Dest.init(convertUtf8ToWString(Buffer),true);
}
