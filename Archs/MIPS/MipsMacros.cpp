#include "stdafx.h"
#include "MipsMacros.h"
#include "CMipsInstruction.h"
#include "Core/Common.h"
#include "Mips.h"
#include "MipsOpcodes.h"
#include "Parser/Parser.h"
#include "MipsParser.h"

MipsMacroCommand::MipsMacroCommand(CAssemblerCommand* content, int macroFlags)
{
	this->content = content;
	this->macroFlags = macroFlags;
	IgnoreLoadDelay = Mips.GetIgnoreDelay();
}

MipsMacroCommand::~MipsMacroCommand()
{
	delete content;
}

bool MipsMacroCommand::Validate()
{
	u64 memoryPos = g_fileManager->getVirtualAddress();
	content->applyFileInfo();
	bool result = content->Validate();
	u64 newMemoryPos = g_fileManager->getVirtualAddress();

	applyFileInfo();

	if (IgnoreLoadDelay == false && Mips.GetDelaySlot() == true && (newMemoryPos-memoryPos) > 4
		&& (macroFlags & MIPSM_DONTWARNDELAYSLOT) == 0)
	{
		Logger::queueError(Logger::Warning,L"Macro with multiple opcodes used inside a delay slot");
	}

	if (newMemoryPos == memoryPos)
		Logger::queueError(Logger::Warning,L"Empty macro content");

	return result;
}

void MipsMacroCommand::Encode() const
{
	content->Encode();
}

void MipsMacroCommand::writeTempData(TempData& tempData) const
{
	content->writeTempData(tempData);
}

std::wstring preprocessMacro(const wchar_t* text, MipsImmediateData& immediates)
{
	// A macro is turned into a sequence of opcodes that are parsed seperately.
	// Any expressions used in the macro may be evaluated at a different memory
	// position, so the '.' operator needs to be replaced by a label at the start
	// of the macro
	std::wstring labelName = L"@@" + Global.symbolTable.getUniqueLabelName();
	immediates.primary.expression.replaceMemoryPos(labelName);
	immediates.secondary.expression.replaceMemoryPos(labelName);

	return formatString(L"%s: %s",labelName,text);
}

CAssemblerCommand* createMacro(Parser& parser, const std::wstring& text, int flags, std::initializer_list<AssemblyTemplateArgument> variables)
{
	CAssemblerCommand* content = parser.parseTemplate(text,variables);
	return new MipsMacroCommand(content,flags);
}

CAssemblerCommand* generateMipsMacroLi(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const wchar_t* templateLi = LR"(
		.if %imm% > 0xFFFF
			.if (%imm% & 0xFFFF8000) == 0xFFFF8000
				.if %lower%
					addiu	%rs%,r0,%imm% & 0xFFFF
				.endif
			.elseif (%imm% & 0xFFFF) == 0
				.if %upper%
					lui	%rs%,%imm% >> 16
				.elseif %lower%
					nop
				.endif
			.else
				.if %upper%
					lui	%rs%,(%imm% >> 16) + ((%imm% & 0x8000) != 0)
				.endif
				.if %lower%
					addiu %rs%,%imm% & 0xFFFF
				.endif
			.endif
		.else
			.if %lower%
				ori	%rs%,r0,%imm%
			.endif
		.endif
	)";

	std::wstring macroText = preprocessMacro(templateLi,immediates);
	return createMacro(parser,macroText,flags, {
			{ L"%upper%",	(flags & MIPSM_UPPER) ? L"1" : L"0" },
			{ L"%lower%",	(flags & MIPSM_LOWER) ? L"1" : L"0" },
			{ L"%rs%",		registers.grs.name },
			{ L"%imm%",		immediates.secondary.expression.toString() },
	});
}

CAssemblerCommand* generateMipsMacroLoadStore(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const wchar_t* templateLoadStore = LR"(
		.if %imm% < 0x8000 || (%imm% & 0xFFFF8000) == 0xFFFF8000
			.if %lower%
				%op%	%rs%,%imm% & 0xFFFF(r0)
			.elseif %upper%
				nop
			.endif
		.else
			.if %upper%
				lui	%temp%,(%imm% >> 16) + ((%imm% & 0x8000) != 0)
			.endif
			.if %lower%
				%op%	%rs%,%imm% & 0xFFFF(%temp%)
			.endif
		.endif
	)";

	wchar_t* op;
	bool isCop = false;
	switch (flags & (MIPSM_ACCESSMASK|MIPSM_LOAD|MIPSM_STORE))
	{
	case MIPSM_LOAD|MIPSM_B:	op = L"lb"; break;
	case MIPSM_LOAD|MIPSM_BU:	op = L"lbu"; break;
	case MIPSM_LOAD|MIPSM_HW:	op = L"lh"; break;
	case MIPSM_LOAD|MIPSM_HWU:	op = L"lhu"; break;
	case MIPSM_LOAD|MIPSM_W:	op = L"lw"; break;
	case MIPSM_LOAD|MIPSM_COP1:	op = L"lwc1"; isCop = true; break;
	case MIPSM_LOAD|MIPSM_COP2:	op = L"lwc2"; isCop = true; break;
	case MIPSM_STORE|MIPSM_B:	op = L"sb"; break;
	case MIPSM_STORE|MIPSM_HW:	op = L"sh"; break;
	case MIPSM_STORE|MIPSM_W:	op = L"sw"; break;
	case MIPSM_STORE|MIPSM_COP1:op = L"swc1"; isCop = true; break;
	case MIPSM_STORE|MIPSM_COP2:op = L"swc2"; isCop = true; break;
	default: return nullptr;
	}

	std::wstring macroText = preprocessMacro(templateLoadStore,immediates);

	bool store = (flags & MIPSM_STORE) != 0;
	return createMacro(parser,macroText,flags, {
			{ L"%upper%",	(flags & MIPSM_UPPER) ? L"1" : L"0" },
			{ L"%lower%",	(flags & MIPSM_LOWER) ? L"1" : L"0" },
			{ L"%rs%",		isCop ? registers.frs.name : registers.grs.name },
			{ L"%temp%",	isCop || store ? L"r1" : registers.grs.name },
			{ L"%imm%",		immediates.secondary.expression.toString() },
			{ L"%op%",		op },
	});
}

CAssemblerCommand* generateMipsMacroLoadUnaligned(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const wchar_t* selectedTemplate;

	std::wstring op;
	int type = flags & MIPSM_ACCESSMASK;
	if (type == MIPSM_HW || type == MIPSM_HWU)
	{
		const wchar_t* templateHalfword = LR"(
			.if (%off% < 0x8000) && ((%off%+1) >= 0x8000)
				.error "Immediate offset too big"
			.else
				%op%	r1,%off%+1(%rs%)
				%op%	%rd%,%off%(%rs%)
				sll		r1,8
				or		%rd%,r1
			.endif
		)";
		
		op = type == MIPSM_HWU ? L"lbu" : L"lb";
		selectedTemplate = templateHalfword;
	} else if (type == MIPSM_W)
	{
		const wchar_t* templateWord = LR"(
			.if (%off% < 0x8000) && ((%off%+3) >= 0x8000)
				.error "Immediate offset too big"
			.else
				lwl	%rd%,%off%+3(%rs%)
				lwr	%rd%,%off%(%rs%)
			.endif
		)";

		if (registers.grs.num == registers.grd.num)
		{
			Logger::printError(Logger::Error,L"Cannot use same register as source and destination");
			return new DummyCommand();
		}

		selectedTemplate = templateWord;
	} else {
		return nullptr;
	}

	std::wstring macroText = preprocessMacro(selectedTemplate,immediates);
	return createMacro(parser,macroText,flags, {
			{ L"%rs%",		registers.grs.name },
			{ L"%rd%",		registers.grd.name },
			{ L"%off%",		immediates.primary.expression.toString() },
			{ L"%op%",		op },
	});
}

CAssemblerCommand* generateMipsMacroStoreUnaligned(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const wchar_t* selectedTemplate;

	int type = flags & MIPSM_ACCESSMASK;
	if (type == MIPSM_HW)
	{
		const wchar_t* templateHalfword = LR"(
			.if (%off% < 0x8000) && ((%off%+1) >= 0x8000)
				.error "Immediate offset too big"
			.else
				sb	%rd%,%off%(%rs%)
				srl	r1,%rd%,8
				sb	r1,%off%+1(%rs%)
			.endif
		)";

		selectedTemplate = templateHalfword;
	} else if (type == MIPSM_W)
	{
		const wchar_t* templateWord = LR"(
			.if (%off% < 0x8000) && ((%off%+3) >= 0x8000)
				.error "Immediate offset too big"
			.else
				swl	%rd%,%off%+3(%rs%)
				swr	%rd%,%off%(%rs%)
			.endif
		)";

		if (registers.grs.num == registers.grd.num)
		{
			Logger::printError(Logger::Error,L"Cannot use same register as source and destination");
			return new DummyCommand();
		}

		selectedTemplate = templateWord;
	} else {
		return nullptr;
	}

	std::wstring macroText = preprocessMacro(selectedTemplate,immediates);
	return createMacro(parser,macroText,flags, {
			{ L"%rs%",		registers.grs.name },
			{ L"%rd%",		registers.grd.name },
			{ L"%off%",		immediates.primary.expression.toString() },
	});
}

CAssemblerCommand* generateMipsMacroBranch(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const wchar_t* selectedTemplate;

	bool bne = (flags & MIPSM_NE) != 0;
	bool beq = (flags & MIPSM_EQ) != 0;
	bool beqz = (flags & MIPSM_GE) != 0;
	bool bnez = (flags & MIPSM_LT) != 0;
	bool immediate = (flags & MIPSM_IMM) != 0;

	std::wstring op;
	if (bne || beq)
	{
		const wchar_t* templateNeEq = LR"(
			li		r1,%imm%
			%op%	%rs%,r1,%dest%
		)";

		selectedTemplate = templateNeEq;
		op = bne ? L"bne" : L"beq";
	} else if (immediate && (beqz || bnez))
	{
		const wchar_t* templateImmediate = LR"(
			.if %imm% > 0xFFFF
				li	r1,%imm%
				slt	r1,%rs%,r1
			.else
				slti	r1,%rs%,%imm%
			.endif
			%op%	r1,%dest%
		)";

		selectedTemplate = templateImmediate;
		op = bnez ? L"bnez" : L"beqz";
	} else if (beqz || bnez)
	{
		const wchar_t* templateRegister = LR"(
			slt		r1,%rs%,%rt%
			%op%	r1,%dest%
		)";

		selectedTemplate = templateRegister;
		op = bnez ? L"bnez" : L"beqz";
	} else {
		return nullptr;
	}
	
	std::wstring macroText = preprocessMacro(selectedTemplate,immediates);
	return createMacro(parser,macroText,flags, {
			{ L"%op%",		op},
			{ L"%rs%",		registers.grs.name },
			{ L"%rt%",		registers.grt.name },
			{ L"%imm%",		immediates.primary.expression.toString() },
			{ L"%dest%",	immediates.secondary.expression.toString() },
	});
}

CAssemblerCommand* generateMipsMacroRotate(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	bool left = (flags & MIPSM_LEFT) != 0;
	bool immediate = (flags & MIPSM_IMM) != 0;
	bool psp = Mips.GetVersion() == MARCH_PSP;

	const wchar_t* selectedTemplate;
	if (psp && immediate)
	{
		const wchar_t* templatePspImmediate = LR"(
			.if %left%
				rotr	%rd%,%rs%,32-%amount%
			.else
				rotr	%rd%,%rs%,%amount%
			.endif"
		)";

		selectedTemplate = templatePspImmediate;
	} else if (psp)
	{
		const wchar_t* templatePspRegister = LR"(
			.if %left%
				subu	r1,r0,%rt%
				rotrv	%rd%,%rs%,r1
			.else
				rotrv	%rd%,%rs%,%rt%
			.endif"
		)";

		selectedTemplate = templatePspRegister;
	} else if (immediate)
	{
		const wchar_t* templateImmediate = LR"(
			.if %left%
				srl	r1,%rs%,32-%amount%
				sll	%rd%,%rs%,%amount%
			.else
				sll	r1,%rs%,32-%amount%
				srl	%rd%,%rs%,%amount%
			.endif"
			or	%rd%,r1"
		)";
		
		selectedTemplate = templateImmediate;
	} else {
		const wchar_t* templateRegister = LR"(
			subu	r1,r0,%rt%
			.if %left%
				srlv	r1,%rs%,r1
				sllv	%rd%,%rs%,%rt%
			.else
				sllv	r1,%rs%,r1
				srlv	%rd%,%rs%,%rt%
			.endif
			or	%rd%,r1"
		)";

		selectedTemplate = templateRegister;
	}
	
	std::wstring macroText = preprocessMacro(selectedTemplate,immediates);
	return createMacro(parser,macroText,flags, {
			{ L"%left%",	left ? L"1" : L"0" },
			{ L"%rs%",		registers.grs.name },
			{ L"%rd%",		registers.grd.name },
			{ L"%rt%",		registers.grt.name },
			{ L"%amount%",	immediates.primary.expression.toString() },
	});
}

/* Placeholders
	i = i1 = 16 bit immediate
	I = i2 = 32 bit immediate
	s,t,d = registers */
const MipsMacroDefinition mipsMacros[] = {
	{ L"li",	L"s,I",		&generateMipsMacroLi,				MIPSM_IMM|MIPSM_UPPER|MIPSM_LOWER },
	{ L"li.u",	L"s,I",		&generateMipsMacroLi,				MIPSM_IMM|MIPSM_UPPER },
	{ L"li.l",	L"s,I",		&generateMipsMacroLi,				MIPSM_IMM|MIPSM_LOWER },
	{ L"la",	L"s,I",		&generateMipsMacroLi,				MIPSM_IMM|MIPSM_UPPER|MIPSM_LOWER },
	{ L"la.u",	L"s,I",		&generateMipsMacroLi,				MIPSM_IMM|MIPSM_UPPER },
	{ L"la.l",	L"s,I",		&generateMipsMacroLi,				MIPSM_IMM|MIPSM_LOWER },

	{ L"lb",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_B|MIPSM_UPPER|MIPSM_LOWER },
	{ L"lbu",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_BU|MIPSM_UPPER|MIPSM_LOWER },
	{ L"lh",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_HW|MIPSM_UPPER|MIPSM_LOWER },
	{ L"lhu",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_HWU|MIPSM_UPPER|MIPSM_LOWER },
	{ L"lw",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_W|MIPSM_UPPER|MIPSM_LOWER },
	{ L"lwc1",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_COP1|MIPSM_UPPER|MIPSM_LOWER },
	{ L"lwc2",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_COP2|MIPSM_UPPER|MIPSM_LOWER },
	
	{ L"lb.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_B|MIPSM_UPPER },
	{ L"lbu.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_BU|MIPSM_UPPER },
	{ L"lh.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_HW|MIPSM_UPPER },
	{ L"lhu.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_HWU|MIPSM_UPPER },
	{ L"lw.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_W|MIPSM_UPPER },
	{ L"lwc1.u",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_COP1|MIPSM_UPPER },
	{ L"lwc2.u",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_COP2|MIPSM_UPPER },
	
	{ L"lb.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_B|MIPSM_LOWER },
	{ L"lbu.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_BU|MIPSM_LOWER },
	{ L"lh.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_HW|MIPSM_LOWER },
	{ L"lhu.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_HWU|MIPSM_LOWER },
	{ L"lw.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_W|MIPSM_LOWER },
	{ L"lwc1.l",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_COP1|MIPSM_LOWER },
	{ L"lwc2.l",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_COP2|MIPSM_LOWER },
	
	{ L"ulh",	L"d,i(s)",	&generateMipsMacroLoadUnaligned,	MIPSM_HW|MIPSM_IMM },
	{ L"ulh",	L"d,(s)",	&generateMipsMacroLoadUnaligned,	MIPSM_HW },
	{ L"ulhu",	L"d,i(s)",	&generateMipsMacroLoadUnaligned,	MIPSM_HWU|MIPSM_IMM },
	{ L"ulhu",	L"d,(s)",	&generateMipsMacroLoadUnaligned,	MIPSM_HWU },
	{ L"ulw",	L"d,i(s)",	&generateMipsMacroLoadUnaligned,	MIPSM_W|MIPSM_IMM },
	{ L"ulw",	L"d,(s)",	&generateMipsMacroLoadUnaligned,	MIPSM_W },

	{ L"sb",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_B|MIPSM_UPPER|MIPSM_LOWER },
	{ L"sh",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_HW|MIPSM_UPPER|MIPSM_LOWER },
	{ L"sw",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_W|MIPSM_UPPER|MIPSM_LOWER },
	{ L"swc1",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_COP1|MIPSM_UPPER|MIPSM_LOWER },
	{ L"swc2",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_COP2|MIPSM_UPPER|MIPSM_LOWER },
	
	{ L"sb.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_B|MIPSM_UPPER },
	{ L"sh.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_HW|MIPSM_UPPER },
	{ L"sw.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_W|MIPSM_UPPER },
	{ L"swc1.u",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_COP1|MIPSM_UPPER },
	{ L"swc2.u",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_COP2|MIPSM_UPPER },
	
	{ L"sb.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_B|MIPSM_LOWER },
	{ L"sh.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_HW|MIPSM_LOWER },
	{ L"sw.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_W|MIPSM_LOWER },
	{ L"swc1.l",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_COP1|MIPSM_LOWER },
	{ L"swc2.l",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_COP2|MIPSM_LOWER },

	{ L"ush",	L"d,i(s)",	&generateMipsMacroStoreUnaligned,	MIPSM_HW|MIPSM_IMM },
	{ L"ush",	L"d,(s)",	&generateMipsMacroStoreUnaligned,	MIPSM_HW },
	{ L"usw",	L"d,i(s)",	&generateMipsMacroStoreUnaligned,	MIPSM_W|MIPSM_IMM },
	{ L"usw",	L"d,(s)",	&generateMipsMacroStoreUnaligned,	MIPSM_W },

	{ L"blt",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_LT|MIPSM_DONTWARNDELAYSLOT },
	{ L"blt",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_LT|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ L"bge",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_GE|MIPSM_DONTWARNDELAYSLOT },
	{ L"bge",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_GE|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ L"bne",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_NE|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ L"beq",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_EQ|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },

	{ L"rol",	L"d,s,t",	&generateMipsMacroRotate,			MIPSM_LEFT },
	{ L"rol",	L"d,s,i",	&generateMipsMacroRotate,			MIPSM_LEFT|MIPSM_IMM },
	{ L"ror",	L"d,s,t",	&generateMipsMacroRotate,			MIPSM_RIGHT },
	{ L"ror",	L"d,s,i",	&generateMipsMacroRotate,			MIPSM_RIGHT|MIPSM_IMM },

	{ NULL,		NULL,		NULL,								0 }
};