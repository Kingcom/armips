#include "Archs/MIPS/MipsMacros.h"

#include "Archs/MIPS/CMipsInstruction.h"
#include "Archs/MIPS/Mips.h"
#include "Archs/MIPS/MipsOpcodes.h"
#include "Archs/MIPS/MipsParser.h"
#include "Core/Common.h"
#include "Core/FileManager.h"
#include "Core/Misc.h"
#include "Parser/Parser.h"
#include "Util/Util.h"

MipsMacroCommand::MipsMacroCommand(std::unique_ptr<CAssemblerCommand> content, int macroFlags)
{
	this->content = std::move(content);
	this->macroFlags = macroFlags;
	IgnoreLoadDelay = Mips.GetIgnoreDelay();
}

bool MipsMacroCommand::Validate(const ValidateState &state)
{
	int64_t memoryPos = g_fileManager->getVirtualAddress();
	content->applyFileInfo();
	bool result = content->Validate(state);
	int64_t newMemoryPos = g_fileManager->getVirtualAddress();

	applyFileInfo();

	if (!IgnoreLoadDelay && Mips.GetDelaySlot() && (newMemoryPos-memoryPos) > 4
		&& (macroFlags & MIPSM_DONTWARNDELAYSLOT) == 0)
	{
		Logger::queueError(Logger::Warning, "Macro with multiple opcodes used inside a delay slot");
	}

	if (newMemoryPos == memoryPos)
		Logger::queueError(Logger::Warning, "Empty macro content");

	return result;
}

void MipsMacroCommand::Encode() const
{
	content->Encode();
}

void MipsMacroCommand::writeTempData(TempData& tempData) const
{
	content->applyFileInfo();
	content->writeTempData(tempData);
}

std::string preprocessMacro(const char* text, MipsImmediateData& immediates)
{
	// A macro is turned into a sequence of opcodes that are parsed seperately.
	// Any expressions used in the macro may be evaluated at a different memory
	// position, so the '.' operator needs to be replaced by a label at the start
	// of the macro
	Identifier labelName = Global.symbolTable.getUniqueLabelName(true);
	immediates.primary.expression.replaceMemoryPos(labelName);
	immediates.secondary.expression.replaceMemoryPos(labelName);

	return tfm::format("%s: %s",labelName,text);
}

std::unique_ptr<CAssemblerCommand> createMacro(Parser& parser, const std::string& text, int flags, std::initializer_list<AssemblyTemplateArgument> variables)
{
	std::unique_ptr<CAssemblerCommand> content = parser.parseTemplate(text,variables);
	return std::make_unique<MipsMacroCommand>(std::move(content),flags);
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroAbs(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const char* templateAbs = R"(
		%sraop% 	r1,%rs%,31
		xor 		%rd%,%rs%,r1
		%subop% 	%rd%,%rd%,r1
	)";

	std::string sraop, subop;

	switch (flags & MIPSM_ACCESSMASK)
	{
	case MIPSM_W:	sraop = "sra"; subop = "subu"; break;
	case MIPSM_DW:	sraop = "dsra32"; subop = "dsubu"; break;
	default: return nullptr;
	}

	std::string macroText = preprocessMacro(templateAbs,immediates);
	return createMacro(parser,macroText,flags, {
			{ "%rd%",		registers.grd.name.string() },
			{ "%rs%",		registers.grs.name.string() },
			{ "%sraop%",	sraop },
			{ "%subop%",	subop },
	});
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroLiFloat(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const char* templateLiFloat = R"(
		li 		r1,float(%imm%)
		mtc1	r1,%rs%
	)";

	std::string macroText = preprocessMacro(templateLiFloat,immediates);
	return createMacro(parser,macroText,flags, {
			{ "%imm%",		immediates.secondary.expression.toString() },
			{ "%rs%",		registers.frs.name.string() },
	});
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroLi(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const char* templateLi = R"(
		.if abs(%imm%) > 0xFFFFFFFF
			.error "Immediate value too big"
		.elseif %imm% & ~0xFFFF
			.if (%imm% & 0xFFFF8000) == 0xFFFF8000
				.if %lower%
					addiu	%rs%,r0, lo(%imm%)
				.endif
			.elseif (%imm% & 0xFFFF) == 0
				.if %upper%
					lui		%rs%, hi(%imm%)
				.elseif %lower%
					nop
				.endif
			.else
				.if %upper%
					lui		%rs%, hi(%imm%)
				.endif
				.if %lower%
					addiu 	%rs%, lo(%imm%)
				.endif
			.endif
		.else
			.if %lower%
				ori		%rs%,r0,%imm%
			.endif
		.endif
	)";

	// floats need to be treated as integers, convert them
	if (immediates.secondary.expression.isConstExpression())
	{
		ExpressionValue value = immediates.secondary.expression.evaluate();
		if (value.isFloat())
		{
			int32_t newValue = getFloatBits((float)value.floatValue);
			immediates.secondary.expression = createConstExpression(newValue);
		}
	}

	std::string macroText = preprocessMacro(templateLi,immediates);
	return createMacro(parser,macroText,flags, {
			{ "%upper%",	(flags & MIPSM_UPPER) ? "1" : "0" },
			{ "%lower%",	(flags & MIPSM_LOWER) ? "1" : "0" },
			{ "%rs%",		registers.grs.name.string() },
			{ "%imm%",		immediates.secondary.expression.toString() },
	});
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroLoadStore(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const char* templateLoadStore = R"(
		.if %imm% & ~0xFFFFFFFF
			.error "Address too big"
		.elseif %imm% < 0x8000 || (%imm% & 0xFFFF8000) == 0xFFFF8000
			.if %lower%
				%op%	%rs%, lo(%imm%)(r0)
			.elseif %upper%
				nop
			.endif
		.else
			.if %upper%
				lui		%temp%, hi(%imm%)
			.endif
			.if %lower%
				%op%	%rs%, lo(%imm%)(%temp%)
			.endif
		.endif
	)";

	const char* op;
	bool isCop = false;
	switch (flags & (MIPSM_ACCESSMASK|MIPSM_LOAD|MIPSM_STORE))
	{
	case MIPSM_LOAD|MIPSM_B:		op = "lb"; break;
	case MIPSM_LOAD|MIPSM_BU:		op = "lbu"; break;
	case MIPSM_LOAD|MIPSM_HW:		op = "lh"; break;
	case MIPSM_LOAD|MIPSM_HWU:		op = "lhu"; break;
	case MIPSM_LOAD|MIPSM_W:		op = "lw"; break;
	case MIPSM_LOAD|MIPSM_WU:		op = "lwu"; break;
	case MIPSM_LOAD|MIPSM_DW:		op = "ld"; break;
	case MIPSM_LOAD|MIPSM_LLSCW:	op = "ll"; break;
	case MIPSM_LOAD|MIPSM_LLSCDW:	op = "lld"; break;
	case MIPSM_LOAD|MIPSM_COP1:		op = "lwc1"; isCop = true; break;
	case MIPSM_LOAD|MIPSM_COP2:		op = "lwc2"; isCop = true; break;
	case MIPSM_LOAD|MIPSM_DCOP1:	op = "ldc1"; isCop = true; break;
	case MIPSM_LOAD|MIPSM_DCOP2:	op = "ldc2"; isCop = true; break;
	case MIPSM_STORE|MIPSM_B:		op = "sb"; break;
	case MIPSM_STORE|MIPSM_HW:		op = "sh"; break;
	case MIPSM_STORE|MIPSM_W:		op = "sw"; break;
	case MIPSM_STORE|MIPSM_DW:		op = "sd"; break;
	case MIPSM_STORE|MIPSM_LLSCW:	op = "sc"; break;
	case MIPSM_STORE|MIPSM_LLSCDW:	op = "scd"; break;
	case MIPSM_STORE|MIPSM_COP1:	op = "swc1"; isCop = true; break;
	case MIPSM_STORE|MIPSM_COP2:	op = "swc2"; isCop = true; break;
	case MIPSM_STORE|MIPSM_DCOP1:	op = "sdc1"; isCop = true; break;
	case MIPSM_STORE|MIPSM_DCOP2:	op = "sdc2"; isCop = true; break;
	default: return nullptr;
	}

	std::string macroText = preprocessMacro(templateLoadStore,immediates);

	bool store = (flags & MIPSM_STORE) != 0;
	return createMacro(parser,macroText,flags, {
			{ "%upper%",	(flags & MIPSM_UPPER) ? "1" : "0" },
			{ "%lower%",	(flags & MIPSM_LOWER) ? "1" : "0" },
			{ "%rs%",		isCop ? registers.frs.name.string() : registers.grs.name.string() },
			{ "%temp%",	isCop || store ? "r1" : registers.grs.name.string() },
			{ "%imm%",		immediates.secondary.expression.toString() },
			{ "%op%",		op },
	});
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroLoadUnaligned(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const char* selectedTemplate;

	std::string op, size;
	int type = flags & MIPSM_ACCESSMASK;
	if (type == MIPSM_HW || type == MIPSM_HWU)
	{
		const char* templateHalfword = R"(
			.if (%off% < 0x8000) && ((%off%+1) >= 0x8000)
				.error "Immediate offset too big"
			.else
				%op%	r1,%off%+1(%rs%)
				%op%	%rd%,%off%(%rs%)
				sll		r1,8
				or		%rd%,r1
			.endif
		)";

		op = type == MIPSM_HWU ? "lbu" : "lb";
		selectedTemplate = templateHalfword;
	} else if (type == MIPSM_W || type == MIPSM_DW)
	{
		const char* templateWord = R"(
			.if (%off% < 0x8000) && ((%off%+%size%-1) >= 0x8000)
				.error "Immediate offset too big"
			.else
				%op%l	%rd%,%off%+%size%-1(%rs%)
				%op%r	%rd%,%off%(%rs%)
			.endif
		)";

		if (registers.grs.num == registers.grd.num)
		{
			Logger::printError(Logger::Error, "Cannot use same register as source and destination");
			return std::make_unique<DummyCommand>();
		}

		op = type == MIPSM_W ? "lw" : "ld";
		size = type == MIPSM_W ? "4" : "8";
		selectedTemplate = templateWord;
	} else {
		return nullptr;
	}

	std::string macroText = preprocessMacro(selectedTemplate,immediates);
	return createMacro(parser,macroText,flags, {
			{ "%rs%",		registers.grs.name.string() },
			{ "%rd%",		registers.grd.name.string() },
			{ "%off%",		immediates.primary.expression.toString() },
			{ "%op%",		op },
			{ "%size%",    size },
	});
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroStoreUnaligned(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const char* selectedTemplate;

	std::string op, size;
	int type = flags & MIPSM_ACCESSMASK;
	if (type == MIPSM_HW)
	{
		const char* templateHalfword = R"(
			.if (%off% < 0x8000) && ((%off%+1) >= 0x8000)
				.error "Immediate offset too big"
			.else
				sb		%rd%,%off%(%rs%)
				srl		r1,%rd%,8
				sb		r1,%off%+1(%rs%)
			.endif
		)";

		selectedTemplate = templateHalfword;
	} else if (type == MIPSM_W || type == MIPSM_DW)
	{
		const char* templateWord = R"(
			.if (%off% < 0x8000) && ((%off%+%size%-1) >= 0x8000)
				.error "Immediate offset too big"
			.else
				%op%l	%rd%,%off%+%size%-1(%rs%)
				%op%r	%rd%,%off%(%rs%)
			.endif
		)";

		if (registers.grs.num == registers.grd.num)
		{
			Logger::printError(Logger::Error, "Cannot use same register as source and destination");
			return std::make_unique<DummyCommand>();
		}

		op = type == MIPSM_W ? "sw" : "sd";
		size = type == MIPSM_W ? "4" : "8";
		selectedTemplate = templateWord;
	} else {
		return nullptr;
	}

	std::string macroText = preprocessMacro(selectedTemplate,immediates);
	return createMacro(parser,macroText,flags, {
			{ "%rs%",		registers.grs.name.string() },
			{ "%rd%",		registers.grd.name.string() },
			{ "%off%",		immediates.primary.expression.toString() },
			{ "%op%",		op },
			{ "%size%",	size },
	});
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroBranch(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const char* selectedTemplate;

	int type = flags & MIPSM_CONDITIONMASK;

	bool bne = type == MIPSM_NE;
	bool beq = type == MIPSM_EQ;
	bool beqz = type == MIPSM_GE || type == MIPSM_GEU;
	bool bnez = type == MIPSM_LT || type == MIPSM_LTU;
	bool unsigned_ = type == MIPSM_GEU || type == MIPSM_LTU;
	bool immediate = (flags & MIPSM_IMM) != 0;
	bool likely = (flags & MIPSM_LIKELY) != 0;
	bool revcmp = (flags & MIPSM_REVCMP) != 0;

	std::string op;
	if (bne || beq)
	{
		const char* templateNeEq = R"(
			.if %imm% == 0
				%op%	%rs%,r0,%dest%
			.else
				li		r1,%imm%
				%op%	%rs%,r1,%dest%
			.endif
		)";

		selectedTemplate = templateNeEq;
		if(likely)
			op = bne ? "bnel" : "beql";
		else
			op = bne ? "bne" : "beq";
	} else if (immediate && (beqz || bnez))
	{
		const char* templateImmediate = R"(
			.if %revcmp% && %imm% == 0
				slt%u% 	r1,r0,%rs%
			.elseif %revcmp%
				li		r1,%imm%
				slt%u%	r1,r1,%rs%
			.elseif (%imm% < -0x8000) || (%imm% >= 0x8000)
				li		r1,%imm%
				slt%u%	r1,%rs%,r1
			.else
				slti%u%	r1,%rs%,%imm%
			.endif
			%op%	r1,%dest%
		)";

		selectedTemplate = templateImmediate;
		if(likely)
			op = bnez ? "bnezl" : "beqzl";
		else
			op = bnez ? "bnez" : "beqz";
	} else if (beqz || bnez)
	{
		const char* templateRegister = R"(
			.if %revcmp%
				slt%u%	r1,%rt%,%rs%
			.else
				slt%u%	r1,%rs%,%rt%
			.endif
			%op%	r1,%dest%
		)";

		selectedTemplate = templateRegister;
		if(likely)
			op = bnez ? "bnezl" : "beqzl";
		else
			op = bnez ? "bnez" : "beqz";
	} else {
		return nullptr;
	}
	
	std::string macroText = preprocessMacro(selectedTemplate,immediates);
	return createMacro(parser,macroText,flags, {
			{ "%op%",		op },
			{ "%u%",		unsigned_ ? "u" : " "},
			{ "%revcmp%",	revcmp ? "1" : "0"},
			{ "%rs%",		registers.grs.name.string() },
			{ "%rt%",		registers.grt.name.string() },
			{ "%imm%",		immediates.primary.expression.toString() },
			{ "%dest%",	immediates.secondary.expression.toString() },
	});
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroSet(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const char* selectedTemplate;

	int type = flags & MIPSM_CONDITIONMASK;

	bool ne = type == MIPSM_NE;
	bool eq = type == MIPSM_EQ;
	bool ge = type == MIPSM_GE || type == MIPSM_GEU;
	bool lt = type == MIPSM_LT || type == MIPSM_LTU;
	bool unsigned_ = type == MIPSM_GEU || type == MIPSM_LTU;
	bool immediate = (flags & MIPSM_IMM) != 0;
	bool revcmp = (flags & MIPSM_REVCMP) != 0;

	if (immediate && (ne || eq))
	{
		const char* templateImmediateEqNe = R"(
			.if %imm% & ~0xFFFF
				li		%rd%,%imm%
				xor		%rd%,%rs%,%rd%
			.else
				xori	%rd%,%rs%,%imm%
			.endif
			.if %eq%
				sltiu	%rd%,%rd%,1
			.else
				sltu	%rd%,r0,%rd%
			.endif
		)";

		selectedTemplate = templateImmediateEqNe;
	} else if (ne || eq)
	{
		const char* templateEqNe = R"(
			xor		%rd%,%rs%,%rt%
			.if %eq%
				sltiu	%rd%,%rd%,1
			.else
				sltu	%rd%,r0,%rd%
			.endif
		)";

		selectedTemplate = templateEqNe;
	} else if (immediate && (ge || lt))
	{
		const char* templateImmediateGeLt = R"(
			.if %revcmp% && %imm% == 0
				slt%u%	%rd%,r0,%rs%
			.elseif %revcmp%
				li		%rd%,%imm%
				slt%u%	%rd%,%rd%,%rs%
			.elseif (%imm% < -0x8000) || (%imm% >= 0x8000)
				li		%rd%,%imm%
				slt%u%	%rd%,%rs%,%rd%
			.else
				slti%u%	%rd%,%rs%,%imm%
			.endif
			.if %ge%
				xori	%rd%,%rd%,1
			.endif
		)";

		selectedTemplate = templateImmediateGeLt;
	} else if (ge)
	{
		const char* templateGe = R"(
			.if %revcmp%
				slt%u%	%rd%,%rt%,%rs%
			.else
				slt%u%	%rd%,%rs%,%rt%
			.endif
			xori	%rd%,%rd%,1
		)";

		selectedTemplate = templateGe;
	} else
	{
		return nullptr;
	}

	std::string macroText = preprocessMacro(selectedTemplate,immediates);
	return createMacro(parser,macroText,flags, {
			{ "%u%",		unsigned_ ? "u" : " "},
			{ "%eq%",		eq ? "1" : "0" },
			{ "%ge%",		ge ? "1" : "0" },
			{ "%revcmp%",	revcmp ? "1" : "0" },
			{ "%rd%",		registers.grd.name.string() },
			{ "%rs%",		registers.grs.name.string() },
			{ "%rt%",		registers.grt.name.string() },
			{ "%imm%",		immediates.secondary.expression.toString() },
	});
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroRotate(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	bool left = (flags & MIPSM_LEFT) != 0;
	bool immediate = (flags & MIPSM_IMM) != 0;
	bool psp = Mips.GetVersion() == MARCH_PSP;

	const char* selectedTemplate;
	if (psp && immediate)
	{
		const char* templatePspImmediate = R"(
			.if %amount% != 0
				.if %left%
					rotr	%rd%,%rs%,-%amount%&31
				.else
					rotr	%rd%,%rs%,%amount%
				.endif
			.else
				move	%rd%,%rs%
			.endif
		)";

		selectedTemplate = templatePspImmediate;
	} else if (psp)
	{
		const char* templatePspRegister = R"(
			.if %left%
				negu	r1,%rt%
				rotrv	%rd%,%rs%,r1
			.else
				rotrv	%rd%,%rs%,%rt%
			.endif
		)";

		selectedTemplate = templatePspRegister;
	} else if (immediate)
	{
		const char* templateImmediate = R"(
			.if %amount% != 0
				.if %left%
					srl	r1,%rs%,-%amount%&31
					sll	%rd%,%rs%,%amount%
				.else
					sll	r1,%rs%,-%amount%&31
					srl	%rd%,%rs%,%amount%
				.endif
				or		%rd%,%rd%,r1
			.else
				move	%rd%,%rs%
			.endif
		)";
		
		selectedTemplate = templateImmediate;
	} else {
		const char* templateRegister = R"(
			negu	r1,%rt%
			.if %left%
				srlv	r1,%rs%,r1
				sllv	%rd%,%rs%,%rt%
			.else
				sllv	r1,%rs%,r1
				srlv	%rd%,%rs%,%rt%
			.endif
			or	%rd%,%rd%,r1
		)";

		selectedTemplate = templateRegister;
	}
	
	std::string macroText = preprocessMacro(selectedTemplate,immediates);
	return createMacro(parser,macroText,flags, {
			{ "%left%",	left ? "1" : "0" },
			{ "%rd%",		registers.grd.name.string() },
			{ "%rs%",		registers.grs.name.string() },
			{ "%rt%",		registers.grt.name.string() },
			{ "%amount%",	immediates.primary.expression.toString() },
	});
}

/* Placeholders
	i = i1 = 16 bit immediate
	I = i2 = 32 bit immediate
	s,t,d = registers */
const MipsMacroDefinition mipsMacros[] = {
	{ "abs",     "d,s",     &generateMipsMacroAbs,             MIPSM_W },
	{ "dabs",    "d,s",     &generateMipsMacroAbs,             MIPSM_DW },

	{ "li",      "s,I",     &generateMipsMacroLi,              MIPSM_IMM|MIPSM_UPPER|MIPSM_LOWER },
	{ "li.u",    "s,I",     &generateMipsMacroLi,              MIPSM_IMM|MIPSM_UPPER },
	{ "li.l",    "s,I",     &generateMipsMacroLi,              MIPSM_IMM|MIPSM_LOWER },
	{ "la",      "s,I",     &generateMipsMacroLi,              MIPSM_IMM|MIPSM_UPPER|MIPSM_LOWER },
	{ "la.u",    "s,I",     &generateMipsMacroLi,              MIPSM_IMM|MIPSM_UPPER },
	{ "la.l",    "s,I",     &generateMipsMacroLi,              MIPSM_IMM|MIPSM_LOWER },

	{ "li.s",    "S,I",     &generateMipsMacroLiFloat,         MIPSM_IMM },

	{ "lb",      "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_B|MIPSM_UPPER|MIPSM_LOWER },
	{ "lbu",     "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_BU|MIPSM_UPPER|MIPSM_LOWER },
	{ "lh",      "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_HW|MIPSM_UPPER|MIPSM_LOWER },
	{ "lhu",     "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_HWU|MIPSM_UPPER|MIPSM_LOWER },
	{ "lw",      "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_W|MIPSM_UPPER|MIPSM_LOWER },
	{ "lwu",     "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_WU|MIPSM_UPPER|MIPSM_LOWER },
	{ "ld",      "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_DW|MIPSM_UPPER|MIPSM_LOWER },
	{ "ll",      "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_LLSCW|MIPSM_UPPER|MIPSM_LOWER },
	{ "lld",     "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_LLSCDW|MIPSM_UPPER|MIPSM_LOWER },
	{ "lwc1",    "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_COP1|MIPSM_UPPER|MIPSM_LOWER },
	{ "l.s",     "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_COP1|MIPSM_UPPER|MIPSM_LOWER },
	{ "lwc2",    "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_COP2|MIPSM_UPPER|MIPSM_LOWER },
	{ "ldc1",    "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_DCOP1|MIPSM_UPPER|MIPSM_LOWER },
	{ "l.d",     "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_DCOP1|MIPSM_UPPER|MIPSM_LOWER },
	{ "ldc2",    "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_DCOP2|MIPSM_UPPER|MIPSM_LOWER },

	{ "lb.u",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_B|MIPSM_UPPER },
	{ "lbu.u",   "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_BU|MIPSM_UPPER },
	{ "lh.u",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_HW|MIPSM_UPPER },
	{ "lhu.u",   "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_HWU|MIPSM_UPPER },
	{ "lw.u",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_W|MIPSM_UPPER },
	{ "lwu.u",   "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_WU|MIPSM_UPPER },
	{ "ld.u",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_DW|MIPSM_UPPER },
	{ "ll.u",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_LLSCW|MIPSM_UPPER },
	{ "lld.u",   "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_LLSCDW|MIPSM_UPPER },
	{ "lwc1.u",  "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_COP1|MIPSM_UPPER },
	{ "l.s.u",   "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_COP1|MIPSM_UPPER },
	{ "lwc2.u",  "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_COP2|MIPSM_UPPER },
	{ "ldc1.u",  "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_DCOP1|MIPSM_UPPER },
	{ "l.d.u",   "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_DCOP1|MIPSM_UPPER },
	{ "ldc2.u",  "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_DCOP2|MIPSM_UPPER },

	{ "lb.l",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_B|MIPSM_LOWER },
	{ "lbu.l",   "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_BU|MIPSM_LOWER },
	{ "lh.l",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_HW|MIPSM_LOWER },
	{ "lhu.l",   "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_HWU|MIPSM_LOWER },
	{ "lw.l",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_W|MIPSM_LOWER },
	{ "lwu.l",   "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_WU|MIPSM_LOWER },
	{ "ld.l",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_DW|MIPSM_LOWER },
	{ "ll.l",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_LLSCW|MIPSM_LOWER },
	{ "lld.l",   "s,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_LLSCDW|MIPSM_LOWER },
	{ "lwc1.l",  "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_COP1|MIPSM_LOWER },
	{ "l.s.l",   "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_COP1|MIPSM_LOWER },
	{ "lwc2.l",  "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_COP2|MIPSM_LOWER },
	{ "ldc1.l",  "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_DCOP1|MIPSM_LOWER },
	{ "l.d.l",   "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_DCOP1|MIPSM_LOWER },
	{ "ldc2.l",  "S,I",     &generateMipsMacroLoadStore,       MIPSM_LOAD|MIPSM_DCOP2|MIPSM_LOWER },

	{ "ulh",     "d,i(s)",  &generateMipsMacroLoadUnaligned,   MIPSM_HW|MIPSM_IMM },
	{ "ulh",     "d,(s)",   &generateMipsMacroLoadUnaligned,   MIPSM_HW },
	{ "ulhu",    "d,i(s)",  &generateMipsMacroLoadUnaligned,   MIPSM_HWU|MIPSM_IMM },
	{ "ulhu",    "d,(s)",   &generateMipsMacroLoadUnaligned,   MIPSM_HWU },
	{ "ulw",     "d,i(s)",  &generateMipsMacroLoadUnaligned,   MIPSM_W|MIPSM_IMM },
	{ "ulw",     "d,(s)",   &generateMipsMacroLoadUnaligned,   MIPSM_W },
	{ "uld",     "d,i(s)",  &generateMipsMacroLoadUnaligned,   MIPSM_DW|MIPSM_IMM },
	{ "uld",     "d,(s)",   &generateMipsMacroLoadUnaligned,   MIPSM_DW },

	{ "sb",      "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_B|MIPSM_UPPER|MIPSM_LOWER },
	{ "sh",      "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_HW|MIPSM_UPPER|MIPSM_LOWER },
	{ "sw",      "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_W|MIPSM_UPPER|MIPSM_LOWER },
	{ "sd",      "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_DW|MIPSM_UPPER|MIPSM_LOWER },
	{ "sc",      "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_LLSCW|MIPSM_UPPER|MIPSM_LOWER },
	{ "scd",     "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_LLSCDW|MIPSM_UPPER|MIPSM_LOWER },
	{ "swc1",    "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_COP1|MIPSM_UPPER|MIPSM_LOWER },
	{ "s.s",     "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_COP1|MIPSM_UPPER|MIPSM_LOWER },
	{ "swc2",    "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_COP2|MIPSM_UPPER|MIPSM_LOWER },
	{ "sdc1",    "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_DCOP1|MIPSM_UPPER|MIPSM_LOWER },
	{ "s.d",     "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_DCOP1|MIPSM_UPPER|MIPSM_LOWER },
	{ "sdc2",    "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_DCOP2|MIPSM_UPPER|MIPSM_LOWER },

	{ "sb.u",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_B|MIPSM_UPPER },
	{ "sh.u",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_HW|MIPSM_UPPER },
	{ "sw.u",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_W|MIPSM_UPPER },
	{ "sd.u",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_DW|MIPSM_UPPER },
	{ "sc.u",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_LLSCW|MIPSM_UPPER },
	{ "scd.u",   "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_LLSCDW|MIPSM_UPPER },
	{ "swc1.u",  "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_COP1|MIPSM_UPPER },
	{ "s.s.u",   "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_COP1|MIPSM_UPPER },
	{ "swc2.u",  "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_COP2|MIPSM_UPPER },
	{ "sdc1.u",  "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_DCOP1|MIPSM_UPPER },
	{ "s.d.u",   "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_DCOP1|MIPSM_UPPER },
	{ "sdc2.u",  "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_DCOP2|MIPSM_UPPER },

	{ "sb.l",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_B|MIPSM_LOWER },
	{ "sh.l",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_HW|MIPSM_LOWER },
	{ "sw.l",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_W|MIPSM_LOWER },
	{ "sd.l",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_DW|MIPSM_LOWER },
	{ "sc.l",    "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_LLSCW|MIPSM_LOWER },
	{ "scd.l",   "s,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_LLSCDW|MIPSM_LOWER },
	{ "swc1.l",  "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_COP1|MIPSM_LOWER },
	{ "s.s.l",   "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_COP1|MIPSM_LOWER },
	{ "swc2.l",  "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_COP2|MIPSM_LOWER },
	{ "sdc1.l",  "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_DCOP1|MIPSM_LOWER },
	{ "s.d.l",   "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_DCOP1|MIPSM_LOWER },
	{ "sdc2.l",  "S,I",     &generateMipsMacroLoadStore,       MIPSM_STORE|MIPSM_DCOP2|MIPSM_LOWER },

	{ "ush",     "d,i(s)",  &generateMipsMacroStoreUnaligned,  MIPSM_HW|MIPSM_IMM },
	{ "ush",     "d,(s)",   &generateMipsMacroStoreUnaligned,  MIPSM_HW },
	{ "usw",     "d,i(s)",  &generateMipsMacroStoreUnaligned,  MIPSM_W|MIPSM_IMM },
	{ "usw",     "d,(s)",   &generateMipsMacroStoreUnaligned,  MIPSM_W },
	{ "usd",     "d,i(s)",  &generateMipsMacroStoreUnaligned,  MIPSM_DW|MIPSM_IMM },
	{ "usd",     "d,(s)",   &generateMipsMacroStoreUnaligned,  MIPSM_DW },

	{ "blt",     "s,t,I",   &generateMipsMacroBranch,          MIPSM_LT|MIPSM_DONTWARNDELAYSLOT },
	{ "blt",     "s,i,I",   &generateMipsMacroBranch,          MIPSM_LT|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ "bgt",     "s,t,I",   &generateMipsMacroBranch,          MIPSM_LT|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT },
	{ "bgt",     "s,i,I",   &generateMipsMacroBranch,          MIPSM_LT|MIPSM_IMM|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT },
	{ "bltu",    "s,t,I",   &generateMipsMacroBranch,          MIPSM_LTU|MIPSM_DONTWARNDELAYSLOT },
	{ "bltu",    "s,i,I",   &generateMipsMacroBranch,          MIPSM_LTU|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ "bgtu",    "s,t,I",   &generateMipsMacroBranch,          MIPSM_LTU|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT },
	{ "bgtu",    "s,i,I",   &generateMipsMacroBranch,          MIPSM_LTU|MIPSM_IMM|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT },
	{ "bge",     "s,t,I",   &generateMipsMacroBranch,          MIPSM_GE|MIPSM_DONTWARNDELAYSLOT },
	{ "bge",     "s,i,I",   &generateMipsMacroBranch,          MIPSM_GE|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ "ble",     "s,t,I",   &generateMipsMacroBranch,          MIPSM_GE|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT },
	{ "ble",     "s,i,I",   &generateMipsMacroBranch,          MIPSM_GE|MIPSM_IMM|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT },
	{ "bgeu",    "s,t,I",   &generateMipsMacroBranch,          MIPSM_GEU|MIPSM_DONTWARNDELAYSLOT },
	{ "bgeu",    "s,i,I",   &generateMipsMacroBranch,          MIPSM_GEU|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ "bleu",    "s,t,I",   &generateMipsMacroBranch,          MIPSM_GEU|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT },
	{ "bleu",    "s,i,I",   &generateMipsMacroBranch,          MIPSM_GEU|MIPSM_IMM|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT },
	{ "bne",     "s,i,I",   &generateMipsMacroBranch,          MIPSM_NE|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ "beq",     "s,i,I",   &generateMipsMacroBranch,          MIPSM_EQ|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ "bltl",    "s,t,I",   &generateMipsMacroBranch,          MIPSM_LT|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ "bltl",    "s,i,I",   &generateMipsMacroBranch,          MIPSM_LT|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ "bgtl",    "s,t,I",   &generateMipsMacroBranch,          MIPSM_LT|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ "bgtl",    "s,i,I",   &generateMipsMacroBranch,          MIPSM_LT|MIPSM_IMM|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ "bltul",   "s,t,I",   &generateMipsMacroBranch,          MIPSM_LTU|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ "bltul",   "s,i,I",   &generateMipsMacroBranch,          MIPSM_LTU|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ "bgtul",   "s,t,I",   &generateMipsMacroBranch,          MIPSM_LTU|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ "bgtul",   "s,i,I",   &generateMipsMacroBranch,          MIPSM_LTU|MIPSM_IMM|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ "bgel",    "s,t,I",   &generateMipsMacroBranch,          MIPSM_GE|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ "bgel",    "s,i,I",   &generateMipsMacroBranch,          MIPSM_GE|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ "blel",    "s,t,I",   &generateMipsMacroBranch,          MIPSM_GE|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ "blel",    "s,i,I",   &generateMipsMacroBranch,          MIPSM_GE|MIPSM_IMM|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ "bgeul",   "s,t,I",   &generateMipsMacroBranch,          MIPSM_GEU|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ "bgeul",   "s,i,I",   &generateMipsMacroBranch,          MIPSM_GEU|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ "bleul",   "s,t,I",   &generateMipsMacroBranch,          MIPSM_GEU|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ "bleul",   "s,i,I",   &generateMipsMacroBranch,          MIPSM_GEU|MIPSM_IMM|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ "bnel",    "s,i,I",   &generateMipsMacroBranch,          MIPSM_NE|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ "beql",    "s,i,I",   &generateMipsMacroBranch,          MIPSM_EQ|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },

	{ "slt",     "d,s,I",   &generateMipsMacroSet,             MIPSM_LT|MIPSM_IMM },
	{ "sltu",    "d,s,I",   &generateMipsMacroSet,             MIPSM_LTU|MIPSM_IMM },
	{ "sgt",     "d,s,I",   &generateMipsMacroSet,             MIPSM_LT|MIPSM_IMM|MIPSM_REVCMP },
	{ "sgtu",    "d,s,I",   &generateMipsMacroSet,             MIPSM_LTU|MIPSM_IMM|MIPSM_REVCMP },
	{ "sge",     "d,s,t",   &generateMipsMacroSet,             MIPSM_GE },
	{ "sge",     "d,s,I",   &generateMipsMacroSet,             MIPSM_GE|MIPSM_IMM },
	{ "sle",     "d,s,t",   &generateMipsMacroSet,             MIPSM_GE|MIPSM_REVCMP },
	{ "sle",     "d,s,I",   &generateMipsMacroSet,             MIPSM_GE|MIPSM_IMM|MIPSM_REVCMP },
	{ "sgeu",    "d,s,t",   &generateMipsMacroSet,             MIPSM_GEU },
	{ "sgeu",    "d,s,I",   &generateMipsMacroSet,             MIPSM_GEU|MIPSM_IMM },
	{ "sleu",    "d,s,t",   &generateMipsMacroSet,             MIPSM_GEU|MIPSM_REVCMP },
	{ "sleu",    "d,s,I",   &generateMipsMacroSet,             MIPSM_GEU|MIPSM_IMM|MIPSM_REVCMP },
	{ "sne",     "d,s,t",   &generateMipsMacroSet,             MIPSM_NE },
	{ "sne",     "d,s,I",   &generateMipsMacroSet,             MIPSM_NE|MIPSM_IMM },
	{ "seq",     "d,s,t",   &generateMipsMacroSet,             MIPSM_EQ },
	{ "seq",     "d,s,I",   &generateMipsMacroSet,             MIPSM_EQ|MIPSM_IMM },

	{ "rol",     "d,s,t",   &generateMipsMacroRotate,          MIPSM_LEFT },
	{ "rol",     "d,s,i",   &generateMipsMacroRotate,          MIPSM_LEFT|MIPSM_IMM },
	{ "ror",     "d,s,t",   &generateMipsMacroRotate,          MIPSM_RIGHT },
	{ "ror",     "d,s,i",   &generateMipsMacroRotate,          MIPSM_RIGHT|MIPSM_IMM },

	{ nullptr,   nullptr,   nullptr,                           0 }
};
