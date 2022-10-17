#include "Archs/MIPS/MipsParser.h"

#include "Archs/MIPS/Mips.h"
#include "Archs/MIPS/MipsElfFile.h"
#include "Archs/MIPS/MipsMacros.h"
#include "Archs/MIPS/PsxRelocator.h"
#include "Commands/CDirectiveFile.h"
#include "Core/Common.h"
#include "Parser/DirectivesParser.h"
#include "Parser/ExpressionParser.h"
#include "Parser/Parser.h"
#include "Util/Util.h"

#define CHECK(exp) if (!(exp)) return false;

const MipsRegisterDescriptor mipsRegisters[] = {
	{ "r0", 0 },  { "zero", 0},  { "at", 1 },  { "r1", 1 },
	{ "v0", 2 },  { "r2", 2 },   { "v1", 3 },  { "r3", 3 },
	{ "a0", 4 },  { "r4", 4 },   { "a1", 5 },  { "r5", 5 },
	{ "a2", 6 },  { "r6", 6 },   { "a3", 7 },  { "r7", 7 },
	{ "t0", 8 },  { "r8", 8 },   { "t1", 9 },  { "r9", 9 },
	{ "t2", 10 }, { "r10", 10 }, { "t3", 11 }, { "r11", 11 },
	{ "t4", 12 }, { "r12", 12 }, { "t5", 13 }, { "r13", 13 },
	{ "t6", 14 }, { "r14", 14 }, { "t7", 15 }, { "r15", 15 },
	{ "s0", 16 }, { "r16", 16 }, { "s1", 17 }, { "r17", 17 },
	{ "s2", 18 }, { "r18", 18 }, { "s3", 19 }, { "r19", 19 },
	{ "s4", 20 }, { "r20", 20 }, { "s5", 21 }, { "r21", 21 },
	{ "s6", 22 }, { "r22", 22 }, { "s7", 23 }, { "r23", 23 },
	{ "t8", 24 }, { "r24", 24 }, { "t9", 25 }, { "r25", 25 },
	{ "k0", 26 }, { "r26", 26 }, { "k1", 27 }, { "r27", 27 },
	{ "gp", 28 }, { "r28", 28 }, { "sp", 29 }, { "r29", 29 },
	{ "fp", 30 }, { "r30", 30 }, { "ra", 31 }, { "r31", 31 },
	{ "s8", 30 },
};

const MipsRegisterDescriptor mipsFloatRegisters[] = {
	{ "f0", 0 },   { "f1", 1 },   { "f2", 2 },   { "f3", 3 },
	{ "f4", 4 },   { "f5", 5 },   { "f6", 6 },   { "f7", 7 },
	{ "f8", 8 },   { "f9", 9 },   { "f00", 0 },  { "f01", 1 },
	{ "f02", 2 },  { "f03", 3 },  { "f04", 4 },  { "f05", 5 },
	{ "f06", 6 },  { "f07", 7 },  { "f08", 8 },  { "f09", 9 },
	{ "f10", 10 }, { "f11", 11 }, { "f12", 12 }, { "f13", 13 },
	{ "f14", 14 }, { "f15", 15 }, { "f16", 16 }, { "f17", 17 },
	{ "f18", 18 }, { "f19", 19 }, { "f20", 20 }, { "f21", 21 },
	{ "f22", 22 }, { "f23", 23 }, { "f24", 24 }, { "f25", 25 },
	{ "f26", 26 }, { "f27", 27 }, { "f28", 28 }, { "f29", 29 },
	{ "f30", 30 }, { "f31", 31 },
};

const MipsRegisterDescriptor mipsFpuControlRegisters[] = {
	{ "fir", 0 }, { "fcr0", 0 }, { "fcsr", 31 }, { "fcr31", 31 },
};

const MipsRegisterDescriptor mipsCop0Registers[] = {
	{ "index", 0},      { "random", 1 },    { "entrylo", 2 },
	{ "entrylo0", 2 },  { "entrylo1", 3 },  { "context", 4 },
	{ "pagemask", 5 },  { "wired", 6 },     { "badvaddr", 8 },
	{ "count", 9 },     { "entryhi", 10 },  { "compare", 11 },
	{ "status", 12 },   { "sr", 12 },       { "cause", 13 },
	{ "epc", 14 },      { "prid", 15 },     { "config", 16 },
	{ "lladdr", 17 },   { "watchlo", 18 },  { "watchhi", 19 },
	{ "xcontext", 20 }, { "badpaddr", 23 }, { "ecc", 26 },
	{ "perr", 26},      { "cacheerr", 27 }, { "taglo", 28 },
	{ "taghi", 29 },    { "errorepc", 30 },
};

const MipsRegisterDescriptor mipsPs2Cop2FpRegisters[] = {
	{ "vf0", 0 },   { "vf1", 1 },   { "vf2", 2 },   { "vf3", 3 },
	{ "vf4", 4 },   { "vf5", 5 },   { "vf6", 6 },   { "vf7", 7 },
	{ "vf8", 8 },   { "vf9", 9 },   { "vf00", 0 },  { "vf01", 1 },
	{ "vf02", 2 },  { "vf03", 3 },  { "vf04", 4 },  { "vf05", 5 },
	{ "vf06", 6 },  { "vf07", 7 },  { "vf08", 8 },  { "vf09", 9 },
	{ "vf10", 10 }, { "vf11", 11 }, { "vf12", 12 }, { "vf13", 13 },
	{ "vf14", 14 }, { "vf15", 15 }, { "vf16", 16 }, { "vf17", 17 },
	{ "vf18", 18 }, { "vf19", 19 }, { "vf20", 20 }, { "vf21", 21 },
	{ "vf22", 22 }, { "vf23", 23 }, { "vf24", 24 }, { "vf25", 25 },
	{ "vf26", 26 }, { "vf27", 27 }, { "vf28", 28 }, { "vf29", 29 },
	{ "vf30", 30 }, { "vf31", 31 },
};

const MipsRegisterDescriptor mipsPsxCop2DataRegisters[] = {
	{ "vxy0", 0 },  { "vz0", 1 },   { "vxy1", 2 },  { "vz1", 3 },
	{ "vxy2", 4 },  { "vz2", 5 },   { "rgbc", 6 },  { "otz", 7 },
	{ "ir0", 8 },   { "ir1", 9 },   { "ir2", 10 },  { "ir3", 11 },
	{ "sxy0", 12 }, { "sxy1", 13 }, { "sxy2", 14 }, { "sxyp", 15 },
	{ "sz0", 16 },  { "sz1", 17 },  { "sz2", 18 },  { "sz3", 19 },
	{ "rgb0", 20 }, { "rgb1", 21 }, { "rgb2", 22 }, { "res1", 23 },
	{ "mac0", 24 }, { "mac1", 25 }, { "mac2", 26 }, { "mac3", 27 },
	{ "irgb", 28 }, { "orgb", 29 }, { "lzcs", 30 }, { "lzcr", 31 },
};

const MipsRegisterDescriptor mipsPsxCop2ControlRegisters[] = {
	{ "rt0", 0 },   { "rt1", 1 },   { "rt2", 2 },   { "rt3", 3 },
	{ "rt4", 4 },   { "trx", 5 },   { "try", 6 },   { "trz", 7 },
	{ "llm0", 8 },  { "llm1", 9 },  { "llm2", 10 }, { "llm3", 11 },
	{ "llm4", 12 }, { "rbk", 13 },  { "gbk", 14 },  { "bbk", 15 },
	{ "lcm0", 16 }, { "lcm1", 17 }, { "lcm2", 18 }, { "lcm3", 19 },
	{ "lcm4", 20 }, { "rfc", 21 },  { "gfc", 22 },  { "bfc", 23 },
	{ "ofx", 24 },  { "ofy", 25 },  { "h", 26 },    { "dqa", 27 },
	{ "dqb", 28 },  { "zsf3", 29 }, { "zsf4", 30 }, { "flag", 31 },
};

const MipsRegisterDescriptor mipsRspCop0Registers[] = {
	{ "sp_mem_addr", 0 }, { "sp_dram_addr", 1 }, { "sp_rd_len", 2 },
	{ "sp_wr_len", 3 },   { "sp_status", 4 },    { "sp_dma_full", 5 },
	{ "sp_dma_busy", 6 }, { "sp_semaphore", 7 }, { "dpc_start", 8 },
	{ "dpc_end", 9 },     { "dpc_current", 10 }, { "dpc_status", 11 },
	{ "dpc_clock", 12 },  { "dpc_bufbusy", 13 }, { "dpc_pipebusy", 14 },
	{ "dpc_tmem", 15 },
};

const MipsRegisterDescriptor mipsRspVectorControlRegisters[] = {
	{ "vco", 0 }, { "vcc", 1 }, { "vce", 2 },
};

const MipsRegisterDescriptor mipsRspVectorRegisters[] = {
	{ "v0", 0 },   { "v1", 1 },   { "v2", 2 },   { "v3", 3 },
	{ "v4", 4 },   { "v5", 5 },   { "v6", 6 },   { "v7", 7 },
	{ "v8", 8 },   { "v9", 9 },   { "v00", 0 },  { "v01", 1 },
	{ "v02", 2 },  { "v03", 3 },  { "v04", 4 },  { "v05", 5 },
	{ "v06", 6 },  { "v07", 7 },  { "v08", 8 },  { "v09", 9 },
	{ "v10", 10 }, { "v11", 11 }, { "v12", 12 }, { "v13", 13 },
	{ "v14", 14 }, { "v15", 15 }, { "v16", 16 }, { "v17", 17 },
	{ "v18", 18 }, { "v19", 19 }, { "v20", 20 }, { "v21", 21 },
	{ "v22", 22 }, { "v23", 23 }, { "v24", 24 }, { "v25", 25 },
	{ "v26", 26 }, { "v27", 27 }, { "v28", 28 }, { "v29", 29 },
	{ "v30", 30 }, { "v31", 31 },
};

std::unique_ptr<CAssemblerCommand> parseDirectiveResetDelay(Parser& parser, int flags)
{
	Mips.SetIgnoreDelay(true);
	return std::make_unique<DummyCommand>();
}

std::unique_ptr<CAssemblerCommand> parseDirectiveFixLoadDelay(Parser& parser, int flags)
{
	Mips.SetFixLoadDelay(true);
	return std::make_unique<DummyCommand>();
}

std::unique_ptr<CAssemblerCommand> parseDirectiveLoadElf(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,1,2))
		return nullptr;

	StringLiteral inputName, outputName;
	if (!list[0].evaluateString(inputName,true))
		return nullptr;

	if (list.size() == 2)
	{
		if (!list[1].evaluateString(outputName,true))
			return nullptr;
		return std::make_unique<DirectiveLoadMipsElf>(inputName.path(),outputName.path());
	} else {
		return std::make_unique<DirectiveLoadMipsElf>(inputName.path());
	}
}

std::unique_ptr<CAssemblerCommand> parseDirectiveImportObj(Parser& parser, int flags)
{
	const Token& start = parser.peekToken();

	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,1,2))
		return nullptr;

	StringLiteral inputName;
	if (!list[0].evaluateString(inputName,true))
		return nullptr;
	
	if (list.size() == 2)
	{
		Identifier ctorName;
		if (!list[1].evaluateIdentifier(ctorName))
			return nullptr;
		
		if (Mips.GetVersion() == MARCH_PSX)
		{
			auto attempt = std::make_unique<DirectiveObjImport>(inputName.path(),ctorName);
			if (!attempt->isSuccessfullyImported())
			{
				parser.printError(start, "Constructor not supported for non-ELF PSX libraries");
				return std::make_unique<InvalidCommand>();
			}
			else
			{
				return attempt;
			}
		}

		return std::make_unique<DirectiveObjImport>(inputName.path(),ctorName);
	}

	if (Mips.GetVersion() == MARCH_PSX)
	{
		// Fallback to ELF, if the PSX library importer fails.
		auto psxLib = std::make_unique<DirectivePsxObjImport>(inputName.path());
		if (!psxLib->isSuccessfullyImported())
			return std::make_unique<DirectiveObjImport>(inputName.path());
		else
			return psxLib;
	}
	else
	{
		return std::make_unique<DirectiveObjImport>(inputName.path());
	}
}

const DirectiveMap mipsDirectives = {
	{ ".resetdelay",		{ &parseDirectiveResetDelay,	0 } },
	{ ".fixloaddelay",		{ &parseDirectiveFixLoadDelay,	0 } },
	{ ".loadelf",			{ &parseDirectiveLoadElf,		0 } },
	{ ".importobj",		{ &parseDirectiveImportObj,		0 } },
	{ ".importlib",		{ &parseDirectiveImportObj,		0 } },
};

std::unique_ptr<CAssemblerCommand> MipsParser::parseDirective(Parser& parser)
{
	return parser.parseDirective(mipsDirectives);
}

bool MipsParser::parseRegisterNumber(Parser& parser, MipsRegisterValue& dest, int numValues)
{
	// check for $0 and $1
	if (parser.peekToken().type == TokenType::Dollar)
	{
		const Token& number = parser.peekToken(1);
		if (number.type == TokenType::Integer && number.intValue() < numValues)
		{
			dest.name = Identifier(tfm::format("$%d", number.intValue()));
			dest.num = (int) number.intValue();

			parser.eatTokens(2);
			return true;
		}
	}

	return false;
}

bool MipsParser::parseRegisterTable(Parser& parser, MipsRegisterValue& dest, const MipsRegisterDescriptor* table, size_t count)
{
	int offset = 0;
	bool hasDollar = parser.peekToken().type == TokenType::Dollar;
	if (hasDollar)
		offset = 1;

	const Token &token = parser.peekToken(offset);

	if (token.type != TokenType::Identifier)
		return false;

	const auto &identifier = token.identifierValue();
	for (size_t i = 0; i < count; i++)
	{
		if (identifier == table[i].name)
		{
			dest.name = identifier;
			dest.num = table[i].num;
			parser.eatTokens(hasDollar ? 2 : 1);
			return true;
		}
	}

	return false;
}

bool MipsParser::parseRegister(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::Normal;

	if (parseRegisterNumber(parser, dest, 32))
		return true;

	return parseRegisterTable(parser,dest,mipsRegisters, std::size(mipsRegisters));
}

bool MipsParser::parseFpuRegister(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::Float;

	if (parseRegisterNumber(parser, dest, 32))
		return true;

	return parseRegisterTable(parser,dest,mipsFloatRegisters, std::size(mipsFloatRegisters));
}

bool MipsParser::parseFpuControlRegister(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::FpuControl;

	if (parseRegisterNumber(parser, dest, 32))
		return true;

	return parseRegisterTable(parser,dest,mipsFpuControlRegisters, std::size(mipsFpuControlRegisters));
}

bool MipsParser::parseCop0Register(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::Cop0;

	if (parseRegisterNumber(parser, dest, 32))
		return true;

	return parseRegisterTable(parser,dest,mipsCop0Registers, std::size(mipsCop0Registers));
}

bool MipsParser::parsePs2Cop2Register(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::Ps2Cop2;
	return parseRegisterTable(parser,dest,mipsPs2Cop2FpRegisters, std::size(mipsPs2Cop2FpRegisters));
}

bool MipsParser::parsePsxCop2DataRegister(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::PsxCop2Data;

	if (parseRegisterNumber(parser, dest, 32))
		return true;

	return parseRegisterTable(parser,dest,mipsPsxCop2DataRegisters, std::size(mipsPsxCop2DataRegisters));
}

bool MipsParser::parsePsxCop2ControlRegister(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::PsxCop2Control;

	if (parseRegisterNumber(parser, dest, 32))
		return true;

	return parseRegisterTable(parser,dest,mipsPsxCop2ControlRegisters, std::size(mipsPsxCop2ControlRegisters));
}

bool MipsParser::parseRspCop0Register(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::RspCop0;

	if (parseRegisterNumber(parser, dest, 32))
		return true;

	return parseRegisterTable(parser,dest,mipsRspCop0Registers, std::size(mipsRspCop0Registers));
}

bool MipsParser::parseRspVectorControlRegister(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::RspVectorControl;

	if (parseRegisterNumber(parser, dest, 32))
		return true;

	return parseRegisterTable(parser,dest,mipsRspVectorControlRegisters, std::size(mipsRspVectorControlRegisters));
}

bool MipsParser::parseRspVectorRegister(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::RspVector;
	return parseRegisterTable(parser,dest,mipsRspVectorRegisters, std::size(mipsRspVectorRegisters));
}

bool MipsParser::parseRspVectorElement(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::RspVectorElement;

	if (parser.peekToken().type == TokenType::LBrack)
	{
		static const MipsRegisterDescriptor rspElementNames[] = {
			{ "0q", 2 },  { "1q", 3 }, { "0h", 4 },  { "1h", 5 },
			{ "2h", 6 },  { "3h", 7 }, { "0w", 8 },  { "0", 8 },
			{ "1w", 9 },  { "1", 9 },  { "2w", 10 }, { "2", 10 },
			{ "3w", 11 }, { "3", 11 }, { "4w", 12 }, { "4", 12 },
			{ "5w", 13 }, { "5", 13 }, { "6w", 14 }, { "6", 14 },
			{ "7w", 15 }, { "7", 15 },
		};

		parser.eatToken();

		if (parseRegisterNumber(parser, dest, 16))
			return parser.nextToken().type == TokenType::RBrack;

		const Token& token = parser.nextToken();

		if (token.type != TokenType::Integer && token.type != TokenType::NumberString)
			return false;

		//ignore the numerical values, just use the original text as an identifier
		std::string stringValue = token.getOriginalText();
		if (std::any_of(stringValue.begin(), stringValue.end(), iswupper))
		{
			std::transform(stringValue.begin(), stringValue.end(), stringValue.begin(), tolower);
		}

		for (size_t i = 0; i < std::size(rspElementNames); i++)
		{
			if (stringValue == rspElementNames[i].name)
			{
				dest.num = rspElementNames[i].num;
				dest.name = Identifier(rspElementNames[i].name);

				return parser.nextToken().type == TokenType::RBrack;
			}
		}

		return false;
	}

	dest.num = 0;
	dest.name = Identifier();

	return true;

}

bool MipsParser::parseRspScalarElement(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::RspScalarElement;

	if (parser.nextToken().type != TokenType::LBrack)
		return false;

	const Token &token = parser.nextToken();

	if (token.type != TokenType::Integer || token.intValue() >= 8)
		return false;

	dest.name = Identifier(tfm::format("%d", token.intValue()));
	dest.num = (int)token.intValue() + 8;

	return parser.nextToken().type == TokenType::RBrack;
}

bool MipsParser::parseRspOffsetElement(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::RspOffsetElement;

	if (parser.peekToken().type == TokenType::LBrack)
	{
		parser.eatToken();

		const Token &token = parser.nextToken();

		if (token.type != TokenType::Integer || token.intValue() >= 16)
			return false;

		dest.name = Identifier(tfm::format("%d", token.intValue()));
		dest.num = (int)token.intValue();

		return parser.nextToken().type == TokenType::RBrack;
	}

	dest.num = 0;
	dest.name = Identifier();

	return true;
}

static bool decodeDigit(char digit, int& dest)
{
	if (digit >= '0' && digit <= '9')
	{
		dest = digit-'0';
		return true;
	}
	return false;
}

bool MipsParser::parseVfpuRegister(Parser& parser, MipsRegisterValue& reg, int size)
{
	const Token& token = parser.peekToken();
	if (token.type != TokenType::Identifier || token.identifierValue().size() != 4)
		return false;

	const Identifier &identifier = token.identifierValue();

	int mtx,col,row;
	if (!decodeDigit(identifier.string()[1],mtx)) return false;
	if (!decodeDigit(identifier.string()[2],col)) return false;
	if (!decodeDigit(identifier.string()[3],row)) return false;
	int mode = tolower(identifier.string()[0]);

	if (size < 0 || size > 3)
		return false;

	if (row > 3 || col > 3 || mtx > 7)
		return false;

	reg.num = 0;
	switch (mode)
	{
	case 'r':					// transposed vector
		reg.num |= (1 << 5);
		std::swap(col,row);
		[[fallthrough]];
	case 'c':					// vector	
		reg.type = MipsRegisterType::VfpuVector;

		switch (size)
		{
		case 1:	// pair
		case 3: // quad
			if (row & 1)
				return false;
			break;
		case 2:	// triple
			if (row & 2)
				return false;
			row <<= 1;
			break;
		default:
			return false;
		}
		break;
	case 's':					// single
		reg.type = MipsRegisterType::VfpuVector;

		if (size != 0)
			return false;
		break;
	case 'e':					// transposed matrix
		reg.num |= (1 << 5);
		[[fallthrough]];
	case 'm':					// matrix
		reg.type = MipsRegisterType::VfpuMatrix;

		// check size
		switch (size)
		{
		case 1:	// 2x2
		case 3:	// 4x4
			if (row & 1)
				return false;
			break;
		case 2:	// 3x3
			if (row & ~1)
				return false;
			row <<= 1;
			break;
		default:
			return false;
		}
		break;
	default:
		return false;
	}

	reg.num |= mtx << 2;
	reg.num |= col;
	reg.num |= row << 5;

	reg.name = identifier;
	parser.eatToken();
	return true;
}

bool MipsParser::parseVfpuControlRegister(Parser& parser, MipsRegisterValue& reg)
{
	static const char* vfpuCtrlNames[16] = {
		"spfx", "tpfx", "dpfx", "cc",
		"inf4", "rsv5", "rsv6", "rev",
		"rcx0", "rcx1", "rcx2", "rcx3",
		"rcx4", "rcx5", "rcx6", "rcx7",
	};

	const Token& token = parser.peekToken();

	if (token.type == TokenType::Identifier)
	{
		const auto &identifier = token.identifierValue();
		for (int i = 0; i < 16; i++)
		{
			if (identifier == vfpuCtrlNames[i])
			{
				reg.num = i;
				reg.name = identifier;

				parser.eatToken();
				return true;
			}
		}
	} else if (token.type == TokenType::Integer && token.intValue() <= 15)
	{
		reg.num = (int) token.intValue();
		reg.name = Identifier(vfpuCtrlNames[reg.num]);

		parser.eatToken();
		return true;
	}

	return false;
}

bool MipsParser::parseImmediate(Parser& parser, Expression& dest)
{
	// check for (reg) or reg sequence
	TokenizerPosition pos = parser.getTokenizer()->getPosition();

	bool hasParen = parser.peekToken().type == TokenType::LParen;
	if (hasParen)
		parser.eatToken();

	MipsRegisterValue tempValue;
	bool isRegister = parseRegister(parser,tempValue);
	parser.getTokenizer()->setPosition(pos);

	if (isRegister)
		return false;

	dest = parser.parseExpression();
	return dest.isLoaded();
}

bool MipsParser::matchSymbol(Parser& parser, char symbol)
{
	switch (symbol)
	{
	case '(':
		return parser.matchToken(TokenType::LParen);
	case ')':
		return parser.matchToken(TokenType::RParen);
	case ',':
		return parser.matchToken(TokenType::Comma);
	}

	return false;
}

bool MipsParser::parseVcstParameter(Parser& parser, int& result)
{
	static TokenSequenceParser sequenceParser;

	// initialize on first use
	if (sequenceParser.getEntryCount() == 0)
	{
		// maxfloat
		sequenceParser.addEntry(1,
			{TokenType::Identifier},
			{"maxfloat"}
		);
		// sqrt(2)
		sequenceParser.addEntry(2,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen},
			{"sqrt", INT64_C(2)}
		);
		// sqrt(1/2)
		sequenceParser.addEntry(3,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::Div, TokenType::Integer, TokenType::RParen},
			{"sqrt", INT64_C(1), INT64_C(2)}
		);
		// sqrt(0.5)
		sequenceParser.addEntry(3,
			{TokenType::Identifier, TokenType::LParen, TokenType::Float, TokenType::RParen},
			{"sqrt", 0.5}
		);
		// 2/sqrt(pi)
		sequenceParser.addEntry(4,
			{TokenType::Integer, TokenType::Div, TokenType::Identifier, TokenType::LParen, TokenType::Identifier, TokenType::RParen},
			{INT64_C(2), "sqrt", "pi"}
		);
		// 2/pi
		sequenceParser.addEntry(5,
			{TokenType::Integer, TokenType::Div, TokenType::Identifier},
			{INT64_C(2), "pi"}
		);
		// 1/pi
		sequenceParser.addEntry(6,
			{TokenType::Integer, TokenType::Div, TokenType::Identifier},
			{INT64_C(1), "pi"}
		);
		// pi/4
		sequenceParser.addEntry(7,
			{TokenType::Identifier, TokenType::Div, TokenType::Integer},
			{"pi", INT64_C(4)}
		);
		// pi/2
		sequenceParser.addEntry(8,
			{TokenType::Identifier, TokenType::Div, TokenType::Integer},
			{"pi", INT64_C(2)}
		);
		// pi/6 - early because "pi" is a prefix of it
		sequenceParser.addEntry(16,
			{TokenType::Identifier, TokenType::Div, TokenType::Integer},
			{"pi", INT64_C(6)}
		);
		// pi
		sequenceParser.addEntry(9,
			{TokenType::Identifier},
			{"pi"}
		);
		// e
		sequenceParser.addEntry(10,
			{TokenType::Identifier},
			{"e"}
		);
		// log2(e)
		sequenceParser.addEntry(11,
			{TokenType::Identifier, TokenType::LParen, TokenType::Identifier, TokenType::RParen},
			{"log2", "e"}
		);
		// log10(e)
		sequenceParser.addEntry(12,
			{TokenType::Identifier, TokenType::LParen, TokenType::Identifier, TokenType::RParen},
			{"log10", "e"}
		);
		// ln(2)
		sequenceParser.addEntry(13,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen},
			{"ln", INT64_C(2)}
		);
		// ln(10)
		sequenceParser.addEntry(14,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen},
			{"ln", INT64_C(10)}
		);
		// 2*pi
		sequenceParser.addEntry(15,
			{TokenType::Integer, TokenType::Mult, TokenType::Identifier},
			{INT64_C(2), "pi"}
		);
		// log10(2)
		sequenceParser.addEntry(17,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen},
			{"log10", INT64_C(2)}
		);
		// log2(10)
		sequenceParser.addEntry(18,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen},
			{"log2", INT64_C(10)}
		);
		// sqrt(3)/2
		sequenceParser.addEntry(19,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen, TokenType::Div, TokenType::Integer},
			{"sqrt", INT64_C(3), INT64_C(2)}
		);
	}

	return sequenceParser.parse(parser,result);
}

bool MipsParser::parseVfpuVrot(Parser& parser, int& result, int size)
{
	int sin = -1;
	int cos = -1;
	bool negSine = false;
	int sineCount = 0;

	if (parser.nextToken().type != TokenType::LBrack)
		return false;
	
	int numElems = size+1;
	for (int i = 0; i < numElems; i++)
	{
		const Token* tokenFinder = &parser.nextToken();
		
		if (i != 0)
		{
			if (tokenFinder->type != TokenType::Comma)
				return false;

			tokenFinder = &parser.nextToken();
		}

		bool isNeg = tokenFinder->type == TokenType::Minus;
		if (isNeg)
			tokenFinder = &parser.nextToken();

		const Token& token = *tokenFinder;

		if (token.type != TokenType::Identifier || token.identifierValue().size() != 1)
			return false;

		const std::string &stringValue = token.identifierValue().string();
		switch (stringValue[0])
		{
		case 's':
			// if one is negative, all have to be
			if ((!isNeg && negSine) || (isNeg && !negSine && sineCount > 0))
				return false;

			negSine = negSine || isNeg;
			sin = i;
			sineCount++;
			break;
		case 'c':
			// can't be negative, or happen twice
			if (isNeg || cos != -1)
				return false;
			cos = i;
			break;
		case '0':
			if (isNeg)
				return false;
			break;
		default:
			return false;
		}
	}
	
	if (parser.nextToken().type != TokenType::RBrack)
		return false;
	
	result = negSine ? 0x10 : 0;

	if (sin == -1 && cos == -1)
	{
		return false;
	} else if (sin == -1)
	{
		if (numElems == 4)
			return false;
		
		result |= cos;
		result |= ((size+1) << 2);
	} else if (cos == -1)
	{
		if (numElems == 4)
			return false;

		if (sineCount == 1)
		{
			result |= (size+1);
			result |= (sin << 2);
		} else if (sineCount == numElems)
		{
			result |= (size+1);
			result |= ((size+1) << 2);
		} else {
			return false;
		}
	} else {
		if (sineCount > 1)
		{
			if (sineCount+1 != numElems)
				return false;
			
			result |= cos;
			result |= (cos << 2);
		} else {
			result |= cos;
			result |= (sin << 2);
		}
	}

	return true;
}

bool MipsParser::parseVfpuCondition(Parser& parser, int& result)
{
	static const char* conditions[] = {
		"fl", "eq", "lt", "le", "tr", "ne", "ge", "gt",
		"ez", "en", "ei", "es", "nz", "nn", "ni", "ns"
	};

	const Token& token = parser.nextToken();
	if (token.type != TokenType::Identifier)
		return false;

	const Identifier &stringValue = token.identifierValue();
	for (size_t i = 0; i <  std::size(conditions); i++)
	{
		if (stringValue == conditions[i])
		{
			result = (int)i;
			return true;
		}
	}

	return false;
}

bool MipsParser::parseVpfxsParameter(Parser& parser, int& result)
{
	static TokenSequenceParser sequenceParser;

	// initialize on first use
	if (sequenceParser.getEntryCount() == 0)
	{
		// 0
		sequenceParser.addEntry(0, {TokenType::Integer}, {INT64_C(0)} );
		// 1
		sequenceParser.addEntry(1, {TokenType::Integer}, {INT64_C(1)} );
		// 2
		sequenceParser.addEntry(2, {TokenType::Integer}, {INT64_C(2)} );
		// 1/2
		sequenceParser.addEntry(3, {TokenType::Integer, TokenType::Div, TokenType::Integer}, {INT64_C(1), INT64_C(2)} );
		// 3
		sequenceParser.addEntry(4, {TokenType::Integer}, {INT64_C(3)} );
		// 1/3
		sequenceParser.addEntry(5, {TokenType::Integer, TokenType::Div, TokenType::Integer}, {INT64_C(1), INT64_C(3)} );
		// 1/4
		sequenceParser.addEntry(6, {TokenType::Integer, TokenType::Div, TokenType::Integer}, {INT64_C(1), INT64_C(4)} );
		// 1/6
		sequenceParser.addEntry(7, {TokenType::Integer, TokenType::Div, TokenType::Integer}, {INT64_C(1), INT64_C(6)} );
	}

	if (parser.nextToken().type != TokenType::LBrack)
		return false;
	
	for (int i = 0; i < 4; i++)
	{
		const Token *tokenFinder = &parser.nextToken();

		if (i != 0)
		{
			if (tokenFinder->type != TokenType::Comma)
				return false;

			tokenFinder = &parser.nextToken();
		}
		
		// negation
		if (tokenFinder->type == TokenType::Minus)
		{
			result |= 1 << (16+i);
			tokenFinder = &parser.nextToken();
		}

		// abs
		bool abs = false;
		if (tokenFinder->type == TokenType::BitOr)
		{
			result |= 1 << (8+i);
			abs = true;
			tokenFinder = &parser.nextToken();
		}

		const Token& token = *tokenFinder;
		if (token.type != TokenType::Identifier)
			return false;

		// check for register
		const char* reg;
		static const char* vpfxstRegisters = "xyzw";
		const std::string &stringValue = token.identifierValue().string();
		if (stringValue.size() == 1 && (reg = strchr(vpfxstRegisters,stringValue[0])) != nullptr)
		{
			result |= (reg-vpfxstRegisters) << (i*2);

			if (abs && parser.nextToken().type != TokenType::BitOr)
				return false;

			continue;
		}
		
		// abs is invalid with constants
		if (abs)
			return false;

		result |= 1 << (12+i);

		int constNum = -1;
		if (!sequenceParser.parse(parser,constNum))
			return false;
		
		result |= (constNum & 3) << (i*2);
		if (constNum & 4)
			result |= 1 << (8+i);
	}

	return parser.nextToken().type == TokenType::RBrack;
}

bool MipsParser::parseVpfxdParameter(Parser& parser, int& result)
{
	static TokenSequenceParser sequenceParser;

	// initialize on first use
	if (sequenceParser.getEntryCount() == 0)
	{
		// 0-1
		sequenceParser.addEntry(1,
			{TokenType::Integer, TokenType::Minus, TokenType::Integer},
			{INT64_C(0), INT64_C(1)} );
		// 0-1
		sequenceParser.addEntry(-1,
			{TokenType::Integer, TokenType::Minus, TokenType::NumberString},
			{INT64_C(0), "1m"} );
		// 0:1
		sequenceParser.addEntry(1,
			{TokenType::Integer, TokenType::Colon, TokenType::Integer},
			{INT64_C(0), INT64_C(1)} );
		// 0:1
		sequenceParser.addEntry(-1,
			{TokenType::Integer, TokenType::Colon, TokenType::NumberString},
			{INT64_C(0), "1m"} );
		// -1-1
		sequenceParser.addEntry(3,
			{TokenType::Minus, TokenType::Integer, TokenType::Minus, TokenType::Integer},
			{INT64_C(1), INT64_C(1)} );
		// -1-1m
		sequenceParser.addEntry(-3,
			{TokenType::Minus, TokenType::Integer, TokenType::Minus, TokenType::NumberString},
			{INT64_C(1), "1m"} );
		// -1:1
		sequenceParser.addEntry(3,
			{TokenType::Minus, TokenType::Integer, TokenType::Colon, TokenType::Integer},
			{INT64_C(1), INT64_C(1)} );
		// -1:1m
		sequenceParser.addEntry(-3,
			{TokenType::Minus, TokenType::Integer, TokenType::Colon, TokenType::NumberString},
			{INT64_C(1), "1m"} );
	}

	for (int i = 0; i < 4; i++)
	{
		if (i != 0)
		{
			if (parser.nextToken().type != TokenType::Comma)
				return false;
		}

		parser.eatToken();
		
		int num = 0;
		if (!sequenceParser.parse(parser,num))
			return false;

		// m versions
		if (num < 0)
		{
			result |= 1 << (8+i);
			num = abs(num);
		}

		result |= num << (2*i);
	}
	
	return parser.nextToken().type == TokenType::RBrack;
}


bool MipsParser::decodeCop2BranchCondition(const std::string& text, size_t& pos, int& result)
{
	if (pos+3 == text.size())
	{
		if (startsWith(text,"any",pos))
		{
			result = 4;
			pos += 3;
			return true;
		}
		if (startsWith(text,"all",pos))
		{
			result = 5;
			pos += 3;
			return true;
		}
	} else if (pos+1 == text.size())
	{
		switch (text[pos++])
		{
		case 'x':
		case '0':
			result = 0;
			return true;
		case 'y':
		case '1':
			result = 1;
			return true;
		case 'z':
		case '2':
			result = 2;
			return true;
		case 'w':
		case '3':
			result = 3;
			return true;
		case '4':
			result = 4;
			return true;
		case '5':
			result = 5;
			return true;
		}

		// didn't match it
		pos--;
	}

	return false;
}

bool MipsParser::parseCop2BranchCondition(Parser& parser, int& result)
{
	const Token& token = parser.nextToken();

	if (token.type == TokenType::Integer)
	{
		result = (int) token.intValue();
		return token.intValue() <= 5;
	}

	if (token.type != TokenType::Identifier)
		return false;

	size_t pos = 0;
	return decodeCop2BranchCondition(token.identifierValue().string(),pos,result);
}

bool MipsParser::parseWb(Parser& parser)
{
	const Token& token = parser.nextToken();
	if (token.type != TokenType::Identifier)
		return false;

	return token.identifierValue() == "wb";
}

static bool decodeImmediateSize(const char*& encoding, MipsImmediateType& dest)
{
	if (*encoding == 'h')	// half float
	{
		encoding++;
		dest = MipsImmediateType::ImmediateHalfFloat;
	} else {
		int num = 0;
		while (*encoding >= '0' && *encoding <= '9')
		{
			num = num*10 + *encoding-'0';
			encoding++;
		}

		switch (num)
		{
		case 5:
			dest = MipsImmediateType::Immediate5;
			break;
		case 7:
			dest = MipsImmediateType::Immediate7;
			break;
		case 10:
			dest = MipsImmediateType::Immediate10;
			break;
		case 16:
			dest = MipsImmediateType::Immediate16;
			break;
		case 20:
			dest = MipsImmediateType::Immediate20;
			break;
		case 25:
			dest = MipsImmediateType::Immediate25;
			break;
		case 26:
			dest = MipsImmediateType::Immediate26;
			break;
		default:
			return false;
		}
	}

	return true;
}

bool MipsParser::decodeVfpuType(const std::string& name, size_t& pos, int& dest)
{
	if (pos >= name.size())
		return false;

	switch (name[pos++])
	{
	case 's':
		dest = 0;
		return true;
	case 'p':
		dest = 1;
		return true;
	case 't':
		dest = 2;
		return true;
	case 'q':
		dest = 3;
		return true;
	}

	pos--;
	return false;
}

bool MipsParser::decodeOpcode(const std::string& name, const tMipsOpcode& opcode)
{
	const char* encoding = opcode.name;
	size_t pos = 0;

	registers.reset();
	immediate.reset();
	opcodeData.reset();
	hasFixedSecondaryImmediate = false;

	while (*encoding != 0)
	{
		switch (*encoding++)
		{
		case 'S':
			CHECK(decodeVfpuType(name,pos,opcodeData.vfpuSize));
			break;
		case 'B':
			CHECK(decodeCop2BranchCondition(name,pos,immediate.secondary.originalValue));
			immediate.secondary.type = MipsImmediateType::Cop2BranchType;
			immediate.secondary.value = immediate.secondary.originalValue;
			hasFixedSecondaryImmediate = true;
			break;
		default:
			CHECK(pos < name.size());
			CHECK(*(encoding-1) == name[pos++]);
			break;
		}
	}

	return pos >= name.size();
}

void MipsParser::setOmittedRegisters(const tMipsOpcode& opcode)
{
	// copy over omitted registers
	if (opcode.flags & MO_RSD)
		registers.grd = registers.grs;

	if (opcode.flags & MO_RST)
		registers.grt = registers.grs;

	if (opcode.flags & MO_RDT)
		registers.grt = registers.grd;

	if (opcode.flags & MO_FRSD)
		registers.frd = registers.frs;

	if (opcode.flags & MO_RSP_VRSD)
		registers.rspvrd = registers.rspvrs;
}

bool MipsParser::parseParameters(Parser& parser, const tMipsOpcode& opcode)
{
	const char* encoding = opcode.encoding;

	// initialize opcode variables
	immediate.primary.type = MipsImmediateType::None;
	if (!hasFixedSecondaryImmediate)
		immediate.secondary.type = MipsImmediateType::None;

	if (opcodeData.vfpuSize == -1)
	{
		if (opcode.flags & MO_VFPU_SINGLE)
			opcodeData.vfpuSize = 0;
		else if (opcode.flags & MO_VFPU_PAIR)
			opcodeData.vfpuSize = 1;
		else if (opcode.flags & MO_VFPU_TRIPLE)
			opcodeData.vfpuSize = 2;
		else if (opcode.flags & MO_VFPU_QUAD)
			opcodeData.vfpuSize = 3;
	}

	// parse parameters
	MipsRegisterValue tempRegister;
	int actualSize = opcodeData.vfpuSize;

	while (*encoding != 0)
	{
		switch (*encoding++)
		{
		case 't':	// register
			CHECK(parseRegister(parser,registers.grt));
			break;
		case 'd':	// register
			CHECK(parseRegister(parser,registers.grd));
			break;
		case 's':	// register
			CHECK(parseRegister(parser,registers.grs));
			break;
		case 'T':	// float register
			CHECK(parseFpuRegister(parser,registers.frt));
			break;
		case 'D':	// float register
			CHECK(parseFpuRegister(parser,registers.frd));
			break;
		case 'S':	// float register
			CHECK(parseFpuRegister(parser,registers.frs));
			break;
		case 'f':	// fpu control register
			CHECK(parseFpuControlRegister(parser,registers.frs));
			break;
		case 'z':	// cop0 register
			CHECK(parseCop0Register(parser,registers.grd));
			break;
		case 'v':	// psp vfpu reg
			if (*encoding == 'S')
			{
				encoding++;
				actualSize = 0;
			}

			switch (*encoding++)
			{
			case 's':
				CHECK(parseVfpuRegister(parser,registers.vrs,actualSize));
				CHECK(registers.vrs.type == MipsRegisterType::VfpuVector);
				if (opcode.flags & MO_VFPU_6BIT) CHECK(!(registers.vrs.num & 0x40));
				break;
			case 't':
				CHECK(parseVfpuRegister(parser,registers.vrt,actualSize));
				CHECK(registers.vrt.type == MipsRegisterType::VfpuVector);
				if (opcode.flags & MO_VFPU_6BIT) CHECK(!(registers.vrt.num & 0x40));
				break;
			case 'd':
				CHECK(parseVfpuRegister(parser,registers.vrd,actualSize));
				CHECK(registers.vrd.type == MipsRegisterType::VfpuVector);
				if (opcode.flags & MO_VFPU_6BIT) CHECK(!(registers.vrd.num & 0x40));
				break;
			case 'c':
				CHECK(parseVfpuControlRegister(parser,registers.vrd));
				break;
			default:
				return false;
			}
			break;
		case 'm':	// vfpu matrix register
			switch (*encoding++)
			{
			case 's':
				CHECK(parseVfpuRegister(parser,registers.vrs,opcodeData.vfpuSize));
				CHECK(registers.vrs.type == MipsRegisterType::VfpuMatrix);
				if (opcode.flags & MO_VFPU_TRANSPOSE_VS)
					registers.vrs.num ^= 0x20;
				break;
			case 't':
				CHECK(parseVfpuRegister(parser,registers.vrt,opcodeData.vfpuSize));
				CHECK(registers.vrt.type == MipsRegisterType::VfpuMatrix);
				break;
			case 'd':
				CHECK(parseVfpuRegister(parser,registers.vrd,opcodeData.vfpuSize));
				CHECK(registers.vrd.type == MipsRegisterType::VfpuMatrix);
				break;
			default:
				return false;
			}
			break;
		case 'V':	// ps2 vector reg
			switch (*encoding++)
			{
			case 't':	// register
				CHECK(parsePs2Cop2Register(parser,registers.ps2vrt));
				break;
			case 'd':	// register
				CHECK(parsePs2Cop2Register(parser,registers.ps2vrd));
				break;
			case 's':	// register
				CHECK(parsePs2Cop2Register(parser,registers.ps2vrs));
				break;
			default:
				return false;
			}
			break;
		case 'g':	// psx cop2 reg
			switch (*encoding++)
			{
			case 't':	// gte data register
				CHECK(parsePsxCop2DataRegister(parser,registers.grt));
				break;
			case 's':	// gte data register
				CHECK(parsePsxCop2DataRegister(parser,registers.grd));
				break;
			case 'c':	// gte control register
				CHECK(parsePsxCop2ControlRegister(parser,registers.grd));
				break;
			default:
				return false;
			}
			break;
		case 'r':	// forced register
			CHECK(parseRegister(parser,tempRegister));
			CHECK(tempRegister.num == *encoding++);
			break;
		case 'R':	// rsp register
			switch (*encoding++)
			{
			case 'z':	// cop0 register
				CHECK(parseRspCop0Register(parser,registers.grd));
				break;
			case 'c':	// vector control register
				CHECK(parseRspVectorControlRegister(parser,registers.grd));
				break;
			case 't':	// vector register
				CHECK(parseRspVectorRegister(parser,registers.rspvrt));
				break;
			case 'd':	// vector register
				CHECK(parseRspVectorRegister(parser,registers.rspvrd));
				break;
			case 's':	// vector register
				CHECK(parseRspVectorRegister(parser,registers.rspvrs));
				break;
			case 'e':	// vector element
				CHECK(parseRspVectorElement(parser,registers.rspve));
				break;
			case 'l':	// scalar element
				CHECK(parseRspScalarElement(parser,registers.rspve));
				break;
			case 'm':	// scalar destination element
				CHECK(parseRspScalarElement(parser,registers.rspvde));
				break;
			case 'o':	// byte offset element
				CHECK(parseRspOffsetElement(parser,registers.rspvealt));
				break;
			default:
				return false;
			}
			break;
		case 'i':	// primary immediate
			CHECK(parseImmediate(parser,immediate.primary.expression));
			CHECK(decodeImmediateSize(encoding,immediate.primary.type));
			break;
		case 'j':	// secondary immediate
			switch (*encoding++)
			{
			case 'c':
				CHECK(parseImmediate(parser,immediate.secondary.expression));
				immediate.secondary.type = MipsImmediateType::CacheOp;
				break;
			case 'e':
				CHECK(parseImmediate(parser,immediate.secondary.expression));
				immediate.secondary.type = MipsImmediateType::Ext;
				break;
			case 'i':
				CHECK(parseImmediate(parser,immediate.secondary.expression));
				immediate.secondary.type = MipsImmediateType::Ins;
				break;
			case 'b':
				CHECK(parseCop2BranchCondition(parser,immediate.secondary.originalValue));
				immediate.secondary.type = MipsImmediateType::Cop2BranchType;
				immediate.secondary.value = immediate.secondary.originalValue;
				break;
			default:
				return false;
			}
			break;
		case 'C':	// vfpu condition
			CHECK(parseVfpuCondition(parser,opcodeData.vectorCondition));
			break;
		case 'W':	// vfpu argument
			switch (*encoding++)
			{
			case 's':
				CHECK(parseVpfxsParameter(parser,immediate.primary.originalValue));
				immediate.primary.value = immediate.primary.originalValue;
				immediate.primary.type = MipsImmediateType::Immediate20_0;
				break;
			case 'd':
				CHECK(parseVpfxdParameter(parser,immediate.primary.originalValue));
				immediate.primary.value = immediate.primary.originalValue;
				immediate.primary.type = MipsImmediateType::Immediate16;
				break;
			case 'c':
				CHECK(parseVcstParameter(parser,immediate.primary.originalValue));
				immediate.primary.value = immediate.primary.originalValue;
				immediate.primary.type = MipsImmediateType::Immediate5;
				break;
			case 'r':
				CHECK(parseVfpuVrot(parser,immediate.primary.originalValue,opcodeData.vfpuSize));
				immediate.primary.value = immediate.primary.originalValue;
				immediate.primary.type = MipsImmediateType::Immediate5;
				break;
			default:
				return false;
			}
			break;
		case 'w':	// 'wb' characters
			CHECK(parseWb(parser));
			break;
		default:
			CHECK(matchSymbol(parser,*(encoding-1)));
			break;
		}
	}

	opcodeData.opcode = opcode;
	setOmittedRegisters(opcode);

	// the next token has to be a separator, else the parameters aren't
	// completely parsed

	return parser.nextToken().type == TokenType::Separator;

}

std::unique_ptr<CMipsInstruction> MipsParser::parseOpcode(Parser& parser)
{
	if (parser.peekToken().type != TokenType::Identifier)
		return nullptr;

	const Token &token = parser.nextToken();

	bool paramFail = false;
	const MipsArchDefinition& arch = mipsArchs[Mips.GetVersion()];
	const Identifier &identifier = token.identifierValue();

	for (int z = 0; MipsOpcodes[z].name != nullptr; z++)
	{
		if ((MipsOpcodes[z].archs & arch.supportSets) == 0)
			continue;
		if ((MipsOpcodes[z].archs & arch.excludeMask) != 0)
			continue;

		if ((MipsOpcodes[z].flags & MO_64BIT) && !(arch.flags & MO_64BIT))
			continue;
		if ((MipsOpcodes[z].flags & MO_FPU) && !(arch.flags & MO_FPU))
			continue;
		if ((MipsOpcodes[z].flags & MO_DFPU) && !(arch.flags & MO_DFPU))
			continue;

		if (decodeOpcode(identifier.string(),MipsOpcodes[z]))
		{
			TokenizerPosition tokenPos = parser.getTokenizer()->getPosition();

			if (parseParameters(parser,MipsOpcodes[z]))
			{
				// success, return opcode
				return std::make_unique<CMipsInstruction>(opcodeData,immediate,registers);
			}

			parser.getTokenizer()->setPosition(tokenPos);
			paramFail = true;
		}
	}

	if (paramFail)
		parser.printError(token, "MIPS parameter failure");
	else
		parser.printError(token, "Invalid MIPS opcode '%s'",identifier);

	return nullptr;
}

bool MipsParser::parseMacroParameters(Parser& parser, const MipsMacroDefinition& macro)
{
	const char* encoding = macro.args;

	while (*encoding != 0)
	{
		switch (*encoding++)
		{
		case 't':	// register
			CHECK(parseRegister(parser,registers.grt));
			break;
		case 'd':	// register
			CHECK(parseRegister(parser,registers.grd));
			break;
		case 's':	// register
			CHECK(parseRegister(parser,registers.grs));
			break;
		case 'S':	// register
			CHECK(parseFpuRegister(parser,registers.frs));
			break;
		case 'i':	// primary immediate
			CHECK(parseImmediate(parser,immediate.primary.expression));
			break;
		case 'I':	// secondary immediate
			CHECK(parseImmediate(parser,immediate.secondary.expression));
			break;
		default:
			CHECK(matchSymbol(parser,*(encoding-1)));
			break;
		}
	}

	// lw rx,imm is a prefix of lw rx,imm(ry)
	if (parser.peekToken().type == TokenType::LParen)
		return false;

	// the next token has to be a separator, else the parameters aren't
	// completely parsed
	return parser.nextToken().type == TokenType::Separator;
}

std::unique_ptr<CAssemblerCommand> MipsParser::parseMacro(Parser& parser)
{
	Tokenizer* tokenizer = parser.getTokenizer();
	TokenizerPosition startPos = tokenizer->getPosition();

	// Cannot be a reference (we eat below.)
	const Token token = parser.peekToken();
	if (token.type != TokenType::Identifier)
		return nullptr;
	
	parser.eatToken();
	const Identifier &identifier = token.identifierValue();
	for (int z = 0; mipsMacros[z].name != nullptr; z++)
	{
		if (identifier == mipsMacros[z].name)
		{
			TokenizerPosition tokenPos = tokenizer->getPosition();

			if (parseMacroParameters(parser,mipsMacros[z]))
			{
				return mipsMacros[z].function(parser,registers,immediate,mipsMacros[z].flags);
			}

			tokenizer->setPosition(tokenPos);
		}
	}

	// no matching macro found, restore state
	tokenizer->setPosition(startPos);
	return nullptr;
}

void MipsOpcodeFormatter::handleOpcodeName(const MipsOpcodeData& opData)
{
	const char* encoding = opData.opcode.name;

	while (*encoding != 0)
	{
		switch (*encoding++)
		{
		case 'S':
			buffer += "sptq"[opData.vfpuSize];
			break;
		case 'B':
			// TODO
			break;
		default:
			buffer += *(encoding-1);
			break;
		}
	}
}

void MipsOpcodeFormatter::handleImmediate(MipsImmediateType type, unsigned int originalValue, unsigned int opcodeFlags)
{
	switch (type)
	{
	case MipsImmediateType::ImmediateHalfFloat:
		buffer += tfm::format("%f", bitsToFloat(originalValue));
		break;
	case MipsImmediateType::Immediate16:
		if (!(opcodeFlags & MO_IPCR) && originalValue & 0x8000)
			buffer += tfm::format("-0x%X", 0x10000-(originalValue & 0xFFFF));
		else
			buffer += tfm::format("0x%X", originalValue);
		break;
	default:
		buffer += tfm::format("0x%X", originalValue);
		break;
	}
}

void MipsOpcodeFormatter::handleOpcodeParameters(const MipsOpcodeData& opData, const MipsRegisterData& regData,
	const MipsImmediateData& immData)
{
	const char* encoding = opData.opcode.encoding;

	MipsImmediateType type;
	while (*encoding != 0)
	{
		switch (*encoding++)
		{
		case 'r':	// forced register
			buffer += tfm::format("r%d",*encoding);
			encoding += 1;
			break;
		case 's':	// register
			buffer += regData.grs.name.string();
			break;
		case 'd':	// register
			buffer += regData.grd.name.string();
			break;
		case 't':	// register
			buffer += regData.grt.name.string();
			break;
		case 'S':	// fpu register
			buffer += regData.frs.name.string();
			break;
		case 'D':	// fpu register
			buffer += regData.frd.name.string();
			break;
		case 'T':	// fpu register
			buffer += regData.frt.name.string();
			break;
		case 'v':	// psp vfpu reg
		case 'm':	// vfpu matrix register
			switch (*encoding++)
			{
			case 'd':
				buffer += regData.vrd.name.string();
				break;
			case 's':
				buffer += regData.vrs.name.string();
				break;
			case 't':
				buffer += regData.vrt.name.string();
				break;
			}
			break;
		case 'V':	// ps2 vector reg
			switch (*encoding++)
			{
			case 'd':
				buffer += regData.ps2vrd.name.string();
				break;
			case 's':
				buffer += regData.ps2vrs.name.string();
				break;
			case 't':
				buffer += regData.ps2vrt.name.string();
				break;
			}
			break;
		case 'i':	// primary immediate
			decodeImmediateSize(encoding,type);
			handleImmediate(immData.primary.type,immData.primary.originalValue,opData.opcode.flags);
			break;
		case 'j':	// secondary immediate
			handleImmediate(immData.secondary.type,immData.secondary.originalValue, opData.opcode.flags);
			encoding++;
			break;
		case 'C':	// vfpu condition
		case 'W':	// vfpu argument
			// TODO
			break;
		case 'w':	// 'wb' characters
			buffer += "wb";
			break;
		default:
			buffer += *(encoding-1);
			break;
		}
	}
}

const std::string& MipsOpcodeFormatter::formatOpcode(const MipsOpcodeData& opData, const MipsRegisterData& regData,
	const MipsImmediateData& immData)
{
	buffer = "   ";
	handleOpcodeName(opData);

	while (buffer.size() < 11)
		buffer += ' ';

	handleOpcodeParameters(opData,regData,immData);
	return buffer;
}
