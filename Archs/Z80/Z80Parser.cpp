#include "Archs/Z80/CZ80Instruction.h"
#include "Archs/Z80/Z80.h"
#include "Archs/Z80/Z80Opcodes.h"
#include "Archs/Z80/Z80Parser.h"
#include "Core/Expression.h"
#include "Parser/DirectivesParser.h"
#include "Parser/Tokenizer.h"

#define CHECK(exp) if (!(exp)) return false;

const Z80RegisterDescriptor Z80Regs8[] = {
	{ L"b", Z80_REG8_B }, { L"c", Z80_REG8_C },
	{ L"d", Z80_REG8_D }, { L"e", Z80_REG8_E },
	{ L"h", Z80_REG8_H }, { L"l", Z80_REG8_L },
	{ L"a", Z80_REG8_A },
};

const Z80RegisterDescriptor Z80Regs16SP[] = {
	{ L"bc", Z80_REG16_BC }, { L"de", Z80_REG16_DE },
	{ L"hl", Z80_REG16_HL }, { L"sp", Z80_REG16_SP },
};

const Z80RegisterDescriptor Z80Regs16AF[] = { // kinda hacky
	{ L"bc", Z80_REG16_BC }, { L"de", Z80_REG16_DE },
	{ L"hl", Z80_REG16_HL }, { L"af", Z80_REG16_AF },
};

const Z80RegisterDescriptor Z80Regs16IXIY[] = { // kinda hacky
	{ L"bc", Z80_REG16_BC }, { L"de", Z80_REG16_DE },
	{ L"ix", Z80_REG16_IX }, { L"iy", Z80_REG16_IY },
	{ L"sp", Z80_REG16_SP },
};

const Z80RegisterDescriptor Z80HLIncDec16[] = {
	{ L"hli", 0 }, { L"hld", 1 },
};

const Z80RegisterDescriptor Z80Conds[] = {
	{ L"nz", Z80_COND_NZ }, { L"z",  Z80_COND_Z },
	{ L"nc", Z80_COND_NC }, { L"c",  Z80_COND_C },
	{ L"po", Z80_COND_PO }, { L"pe", Z80_COND_PE },
	{ L"p",  Z80_COND_P },  { L"m",  Z80_COND_M },
};

const Z80RegisterDescriptor Z80RegsIR[] = {
	{ L"i", Z80_REG_I }, { L"r", Z80_REG_R },
};

const DirectiveMap Z80Directives = { };

bool Z80Parser::isArchCompatible(const tZ80Opcode& opcode)
{
	switch (Z80.GetVersion())
	{
	case Z80ArchType::Z80:
		return (opcode.flags & Z80_Z80);
	case Z80ArchType::Gameboy:
		return opcode.flags & Z80_GAMEBOY;
	case Z80ArchType::Ereader:
		return opcode.flags & Z80_EREADER;
	default:
		return false;
	}
}

std::unique_ptr<CAssemblerCommand> Z80Parser::parseDirective(Parser& parser)
{
	return parser.parseDirective(Z80Directives);
}

bool Z80Parser::parseRegisterTable(Parser& parser, Z80RegisterValue& dest, const Z80RegisterDescriptor* table, size_t count, int allowed)
{
	const Token& token = parser.peekToken();
	CHECK(token.type == TokenType::Identifier);

	const std::wstring stringValue = token.getStringValue();
	for (size_t i = 0; i < count; i++)
	{
		if (allowed & (1 << table[i].num) && stringValue == table[i].name)
		{
			dest.name = stringValue;
			dest.num = table[i].num;
			parser.eatToken();
			return true;
		}
	}

	return false;
}

bool Z80Parser::parseRegister8(Parser& parser, Z80RegisterValue& dest, int allowed)
{
	return parseRegisterTable(parser, dest, Z80Regs8, std::size(Z80Regs8), allowed);
}

bool Z80Parser::parseRegister16SP(Parser& parser, Z80RegisterValue& dest, int allowed)
{
	return parseRegisterTable(parser, dest, Z80Regs16SP, std::size(Z80Regs16SP), allowed);
}

bool Z80Parser::parseRegister16AF(Parser& parser, Z80RegisterValue& dest, int allowed)
{
	return parseRegisterTable(parser, dest, Z80Regs16AF, std::size(Z80Regs16AF), allowed);
}

bool Z80Parser::parseRegisterIR(Parser& parser, Z80RegisterValue& dest)
{
	return parseRegisterTable(parser, dest, Z80RegsIR, std::size(Z80RegsIR), Z80_REG_IR_ALL);
}

bool Z80Parser::parseRegisterAFShadow(Parser& parser)
{
	const Token &token = parser.nextToken();
	CHECK(token.type == TokenType::Identifier);
	CHECK(token.getStringValue() == L"af'");
	
	return true;
}

bool Z80Parser::parseRegisterIXIY(Parser& parser, Z80RegisterValue& dest, int allowed)
{
	return parseRegisterTable(parser, dest, Z80Regs16IXIY, std::size(Z80Regs16IXIY), allowed);
}

bool Z80Parser::parseCondition(Parser& parser, Z80RegisterValue& dest)
{
	int allowed = (Opcode.flags & Z80_JUMP_RELATIVE)
		? Z80_COND_BIT_JR : Z80_COND_BIT_ALL;

	if (Z80.GetVersion() == Z80ArchType::Gameboy)
	{
		allowed = Z80_COND_BIT_GB;
	}
	else if (Z80.GetVersion() == Z80ArchType::Ereader)
	{
		allowed = (Opcode.flags & (Z80_CALL | Z80_JUMP_RELATIVE))
			? Z80_COND_BIT_ER_CALL : Z80_COND_BIT_ER;
	}

	return parseRegisterTable(parser, dest, Z80Conds, std::size(Z80Conds), allowed);
}

bool Z80Parser::parseHLIncDec(Parser& parser, Z80RegisterValue& dest)
{
	CHECK(parser.matchToken(TokenType::LParen));

	// hli / hld
	if (!parseRegisterTable(parser, dest, Z80HLIncDec16, std::size(Z80HLIncDec16), Z80_REG_BIT_ALL))
	{
		// hl+ / hl-
		CHECK(parseRegister16SP(parser, dest, Z80_REG_BIT(Z80_REG16_HL)));

		const Token& token = parser.nextToken();
		if (token.type == TokenType::Plus)
		{
			dest.name = L"hl+";
			dest.num = 0;
		}
		else if (token.type == TokenType::Minus)
		{
			dest.name = L"hl-";
			dest.num = 1;
		}
		else
		{
			return false;
		}

	}

	CHECK(parser.matchToken(TokenType::RParen));

	return true;
}

bool Z80Parser::parseMemoryRegister8(Parser& parser, Z80RegisterValue& dest, int allowed)
{
	CHECK(parser.matchToken(TokenType::LParen));
	CHECK(parseRegister8(parser, dest, allowed));
	CHECK(parser.matchToken(TokenType::RParen));

	return true;
}

bool Z80Parser::parseMemoryRegister16(Parser& parser, Z80RegisterValue& dest, int allowed)
{
	CHECK(parser.matchToken(TokenType::LParen));
	CHECK(parseRegister16SP(parser, dest, allowed));
	CHECK(parser.matchToken(TokenType::RParen));

	return true;
}

bool Z80Parser::parseMemoryImmediate(Parser& parser, Expression& dest)
{
	CHECK(parser.matchToken(TokenType::LParen));
	dest = parser.parseExpression();
	CHECK(dest.isLoaded());
	CHECK(parser.matchToken(TokenType::RParen));

	return true;
}

bool Z80Parser::parseFF00PlusC(Parser& parser, Z80RegisterValue& destReg)
{
	CHECK(parser.matchToken(TokenType::LParen));

	// 0xFF00+ optional
	const Token& token = parser.peekToken();
	if (token.type == TokenType::Integer)
	{
		CHECK(token.intValue == 0xFF00);
		parser.eatToken();

		CHECK(parser.matchToken(TokenType::Plus));
	}

	CHECK(parseRegister8(parser, destReg, Z80_REG_BIT(Z80_REG8_C)));

	CHECK(parser.matchToken(TokenType::RParen));

	return true;
}

bool Z80Parser::parseMemoryIXIY(Parser& parser, Z80RegisterValue& destReg, Expression& destImm)
{
	CHECK(parser.matchToken(TokenType::LParen));

	CHECK(parseRegisterIXIY(parser, destReg, Z80_REG_BIT(Z80_REG16_IX) | Z80_REG_BIT(Z80_REG16_IY)));

	// + optional
	const Token& token = parser.peekToken();
	if (token.type == TokenType::Plus)
	{
		parser.eatToken();

		destImm = parser.parseExpression();
		CHECK(destImm.isLoaded());
	}
	else
	{
		// Treat as +0
		destImm = createConstExpression(0);
	}

	CHECK(parser.matchToken(TokenType::RParen));

	return true;
}

bool Z80Parser::parseSPImmediate(Parser& parser, Z80RegisterValue& destReg, Expression& destImm, bool& isNegative)
{
	isNegative = false;

	CHECK(parseRegister16SP(parser, destReg, Z80_REG_BIT(Z80_REG16_SP)));

	const Token& token = parser.peekToken();
	if (token.type != TokenType::Plus && token.type != TokenType::Minus)
	{
		// Treat as +0
		destImm = createConstExpression(0);
		return true;
	}
	parser.eatToken();
	isNegative = token.type == TokenType::Minus;

	destImm = parser.parseExpression();
	CHECK(destImm.isLoaded());

	return true;
}

bool Z80Parser::parseOpcodeParameter(Parser& parser, unsigned char paramType, Z80RegisterValue& destReg, Expression& destImm, bool& isNegative)
{
	switch (paramType)
	{
	case Z80_PARAM_REG8_MEMHL:
		if (parseRegister8(parser, destReg, Z80_REG8_BIT_ALL))
		{
			return true;
		}
		if (parseMemoryRegister16(parser, destReg, Z80_REG_BIT(Z80_REG16_HL)))
		{
			destReg.num = Z80_REG8_MEMHL;
			return true;
		}
		return false;
	case Z80_PARAM_REG8:
		return parseRegister8(parser, destReg, Z80_REG8_BIT_ALL);
	case Z80_PARAM_REG16_SP:
		return parseRegister16SP(parser, destReg, Z80_REG16_BIT_ALL);
	case Z80_PARAM_REG16_AF:
		return parseRegister16AF(parser, destReg, Z80_REG16_BIT_ALL);
	case Z80_PARAM_A:
		return parseRegister8(parser, destReg, Z80_REG_BIT(Z80_REG8_A));
	case Z80_PARAM_MEMC:
		return parseMemoryRegister8(parser, destReg, Z80_REG_BIT(Z80_REG8_C));
	case Z80_PARAM_MEMBC_MEMDE:
		return parseMemoryRegister16(parser, destReg, Z80_REG_BIT(Z80_REG16_BC) | Z80_REG_BIT(Z80_REG16_DE));
	case Z80_PARAM_HL:
		return parseRegister16SP(parser, destReg, Z80_REG_BIT(Z80_REG16_HL));
	case Z80_PARAM_MEMHL:
		return parseMemoryRegister16(parser, destReg, Z80_REG_BIT(Z80_REG16_HL));
	case Z80_PARAM_MEMSP:
		return parseMemoryRegister16(parser, destReg, Z80_REG_BIT(Z80_REG16_SP));
	case Z80_PARAM_HLI_HLD:
		return parseHLIncDec(parser, destReg);
	case Z80_PARAM_DE:
		return parseRegister16AF(parser, destReg, Z80_REG_BIT(Z80_REG16_DE));
	case Z80_PARAM_SP:
		return parseRegister16SP(parser, destReg, Z80_REG_BIT(Z80_REG16_SP));
	case Z80_PARAM_AF:
		return parseRegister16AF(parser, destReg, Z80_REG_BIT(Z80_REG16_AF));
	case Z80_PARAM_BC_DE_SP:
		return parseRegister16SP(parser, destReg, Z80_REG_BIT(Z80_REG16_BC) | Z80_REG_BIT(Z80_REG16_DE) | Z80_REG_BIT(Z80_REG16_SP));
	case Z80_PARAM_IR:
		return parseRegisterIR(parser, destReg);
	case Z80_PARAM_AF_SHADOW:
		return parseRegisterAFShadow(parser);
	case Z80_PARAM_IX_IY:
		return parseRegisterIXIY(parser, destReg, Z80_REG_BIT(Z80_REG16_IX) | Z80_REG_BIT(Z80_REG16_IY));
	case Z80_PARAM_REG16_IX_IY:
		return parseRegisterIXIY(parser, destReg, Z80_REG16_BIT_ALL);
	case Z80_PARAM_MEMIX_MEMIY:
		return parseMemoryIXIY(parser, destReg, destImm);
	case Z80_PARAM_IMMEDIATE:
		destImm = parser.parseExpression();
		return destImm.isLoaded();
	case Z80_PARAM_MEMIMMEDIATE:
		return parseMemoryImmediate(parser, destImm);
	case Z80_PARAM_FF00_C:
		return parseFF00PlusC(parser, destReg);
	case Z80_PARAM_SP_IMM:
		return parseSPImmediate(parser, destReg, destImm, isNegative);
	case Z80_PARAM_CONDITION:
		return parseCondition(parser, destReg);
	default:
		return false;
	}
}

bool Z80Parser::parseOpcodeParameterList(Parser& parser, Z80OpcodeVariables& vars)
{
	bool isNegative = false;
	if (Opcode.lhs)
	{
		CHECK(parseOpcodeParameter(parser, Opcode.lhs, vars.LeftParam, vars.ImmediateExpression, isNegative));
	}
	if (Opcode.rhs)
	{
		CHECK(parser.matchToken(TokenType::Comma));

		if (Opcode.flags & Z80_HAS_2_IMMEDIATES)
		{
			CHECK(parseOpcodeParameter(parser, Opcode.rhs, vars.RightParam, vars.ImmediateExpression2, isNegative));
		}
		else
		{
			CHECK(parseOpcodeParameter(parser, Opcode.rhs, vars.RightParam, vars.ImmediateExpression, isNegative));
		}
	}
	vars.IsNegative = isNegative;
	
	// ld (hl),(hl) equivalent to halt
	if ((Opcode.flags & Z80_LOAD_REG8_REG8) &&
		vars.LeftParam.num == Z80_REG8_MEMHL && vars.RightParam.num == Z80_REG8_MEMHL)
	{
		return false;
	}

	// Cannot mix ix and iy
	if (Z80_IS_PARAM_IX_IY(Opcode.lhs) && Z80_IS_PARAM_IX_IY(Opcode.rhs))
	{
		if (vars.LeftParam.num == Z80_REG16_IX && vars.RightParam.num == Z80_REG16_IY ||
			vars.LeftParam.num == Z80_REG16_IY && vars.RightParam.num == Z80_REG16_IX)
		return false;
	}

	return true;
}

std::unique_ptr<CZ80Instruction> Z80Parser::parseOpcode(Parser& parser)
{
	if (parser.peekToken().type != TokenType::Identifier)
		return nullptr;

	const Token& token = parser.nextToken();

	Z80OpcodeVariables vars;
	bool paramFail = false;

	const std::wstring stringValue = token.getStringValue();
	for (int z = 0; Z80Opcodes[z].name != nullptr; z++)
	{
		Opcode = Z80Opcodes[z];

		if (!isArchCompatible(Opcode))
			continue;

		if (stringValue == Opcode.name)
		{
			TokenizerPosition tokenPos = parser.getTokenizer()->getPosition();

			if (parseOpcodeParameterList(parser, vars))
			{
				// success, return opcode
				return std::make_unique<CZ80Instruction>(Opcode, vars);
			}

			parser.getTokenizer()->setPosition(tokenPos);
			paramFail = true;
		}
	}

	if (paramFail)
		parser.printError(token, L"%s parameter failure in %S", Z80.GetName(), stringValue);
	else
		parser.printError(token, L"Invalid %s opcode: %S", Z80.GetName(), stringValue);

	return nullptr;
}
