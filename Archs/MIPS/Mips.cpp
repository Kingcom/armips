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
#include "MipsParser.h"

CMipsArchitecture Mips;

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

CMipsArchitecture::CMipsArchitecture()
{
	FixLoadDelay = false;
	IgnoreLoadDelay = false;
	LoadDelay = false;
	LoadDelayRegister = 0;
	DelaySlot = false;
	Version = MARCH_INVALID;
}

CAssemblerCommand* CMipsArchitecture::parseDirective(Tokenizer& tokenizer)
{
	MipsParser parser;
	return parser.parseDirective(tokenizer);
}

CAssemblerCommand* CMipsArchitecture::parseOpcode(Tokenizer& tokenizer)
{
	MipsParser parser;

	CAssemblerCommand* macro = parser.parseMacro(tokenizer);
	if (macro != nullptr)
		return macro;

	return parser.parseOpcode(tokenizer);
}


bool CMipsArchitecture::AssembleDirective(const std::wstring& name, const std::wstring& args)
{
	Logger::printError(Logger::FatalError,L"Unsupported operation");
	return false;
}

void CMipsArchitecture::AssembleOpcode(const std::wstring& name, const std::wstring& args)
{
	Logger::printError(Logger::FatalError,L"Unsupported operation");
	return;
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
