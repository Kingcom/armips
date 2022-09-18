#include "Archs/SuperH/ShElfRelocator.h"

#include "Commands/CAssemblerCommand.h"
#include "Core/Common.h"
#include "Parser/Parser.h"

int ShElfRelocator::expectedMachine() const
{
	return EM_SH2;
}

bool ShElfRelocator::relocateOpcode(int type, const RelocationData& data, std::vector<RelocationAction>& actions, std::vector<std::string>& errors)
{
	unsigned int op = data.opcode;
	bool result = true;

	switch (type)
	{
	case R_SH_DIR32:
		op += (int) (data.relocationBase + data.addend);
		break;
	case R_SH_REL32:
		op += (int) (data.relocationBase - data.opcodeOffset + data.addend); // I have no clue whether this is correct
		break;
	default:
		errors.emplace_back(tfm::format("Unknown SuperH relocation type %d",type));
		return false;
	}

	actions.emplace_back(data.opcodeOffset, op);
	return result;
}

void ShElfRelocator::setSymbolAddress(RelocationData& data, int64_t symbolAddress, int symbolType)
{
	data.symbolAddress = symbolAddress;
	data.targetSymbolType = symbolType;
}

/*const char* shCtorTemplate = R"(
	addiu	sp,-32
	sw		ra,0(sp)
	sw		s0,4(sp)
	sw		s1,8(sp)
	sw		s2,12(sp)
	sw		s3,16(sp)
	li		s0,%ctorTable%
	li		s1,%ctorTable%+%ctorTableSize%
	%outerLoopLabel%:
	lw		s2,(s0)
	lw		s3,4(s0)
	addiu	s0,8
	%innerLoopLabel%:
	lw		a0,(s2)
	jalr	a0
	addiu	s2,4h
	bne		s2,s3,%innerLoopLabel%
	nop
	bne		s0,s1,%outerLoopLabel%
	nop
	lw		ra,0(sp)
	lw		s0,4(sp)
	lw		s1,8(sp)
	lw		s2,12(sp)
	lw		s3,16(sp)
	jr		ra
	addiu	sp,32
	%ctorTable%:
	.word	%ctorContent%
)";*/


//
//	WARNING: NOT TESTED!!!!!
//
const char* shCtorTemplate = R"(
	mov.l	r8,@-r15
	mov.l	r9,@-r15
	mov.l	r10,@-r15
	mov.l	r11,@-r15
	sts.l	pr,@-r15
	mova	%ctorTable%,r0
	mov		r0,r8
	mova	%ctorTable%+%ctorTableSize%,r0
	mov		r0,r9
	%outerLoopLabel%:
	mov.l	@r8,r10
	mov.l	@(4,r9),r11
	add		#8,r8
	%innerLoopLabel%:
	mov.l	@r10,r4
	jsr		r4
	add		#4,r10
	cmp/eq 	r10,r11
	bf		%innerLoopLabel%
	cmp/eq	r8,r9
	bf		%outerLoopLabel%
	lds.l	@r15+,pr
	mov.l	@r15+,r11
	mov.l	@r15+,r10
	mov.l	@r15+,r9
	rts
	mov.l	@r15+,r8
	.align 4
	%ctorTable%:
	.word	%ctorContent%
)";

std::unique_ptr<CAssemblerCommand> ShElfRelocator::generateCtorStub(std::vector<ElfRelocatorCtor>& ctors)
{
	Parser parser;
	if (ctors.size() != 0)
	{
		// create constructor table
		std::string table;
		for (size_t i = 0; i < ctors.size(); i++)
		{
			if (i != 0)
				table += ',';
			table += tfm::format("%s,%s+0x%08X",ctors[i].symbolName,ctors[i].symbolName,ctors[i].size);
		}

		return parser.parseTemplate(shCtorTemplate,{
			{ "%ctorTable%",		Global.symbolTable.getUniqueLabelName().string() },
			{ "%ctorTableSize%",	tfm::format("%d",ctors.size()*8) },
			{ "%outerLoopLabel%",	Global.symbolTable.getUniqueLabelName().string() },
			{ "%innerLoopLabel%",	Global.symbolTable.getUniqueLabelName().string() },
			{ "%ctorContent%",		table },
		});
	} else {
		return parser.parseTemplate("jr ra :: nop");
	}
}
