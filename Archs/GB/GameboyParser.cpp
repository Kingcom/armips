#include "Archs/GB/CGameboyInstruction.h"
#include "Archs/GB/Gameboy.h"
#include "Archs/GB/GameboyOpcodes.h"
#include "Archs/GB/GameboyParser.h"
#include "Core/Expression.h"
#include "Parser/DirectivesParser.h"
#include "Parser/Tokenizer.h"

#define CHECK(exp) if (!(exp)) return false;

const GameboyRegisterDescriptor gameboyRegs8[] = {
	{ L"b", GB_REG8_B }, { L"c", GB_REG8_C },
	{ L"d", GB_REG8_D }, { L"e", GB_REG8_E },
	{ L"h", GB_REG8_H }, { L"l", GB_REG8_L },
	{ L"a", GB_REG8_A },
};

const GameboyRegisterDescriptor gameboyRegs16SP[] = {
	{ L"bc", GB_REG16_BC }, { L"de", GB_REG16_DE },
	{ L"hl", GB_REG16_HL }, { L"sp", GB_REG16_SP },
};

const GameboyRegisterDescriptor gameboyRegs16AF[] = { // kinda hacky
	{ L"bc", GB_REG16_BC }, { L"de", GB_REG16_DE },
	{ L"hl", GB_REG16_HL }, { L"af", GB_REG16_AF },
};

const GameboyRegisterDescriptor gameboyHLIncDec16[] = {
	{ L"hli", 0 }, { L"hld", 1 },
};

const GameboyRegisterDescriptor gameboyConds[] = {
	{ L"nz", GB_COND_NZ }, { L"z", GB_COND_Z },
	{ L"nc", GB_COND_NC }, { L"c", GB_COND_C },
};

const DirectiveMap gameboyDirectives = { };

std::unique_ptr<CAssemblerCommand> GameboyParser::parseDirective(Parser& parser)
{
	return parser.parseDirective(gameboyDirectives);
}

bool GameboyParser::parseRegisterTable(Parser& parser, GameboyRegisterValue& dest, const GameboyRegisterDescriptor* table, size_t count, int allowed)
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

bool GameboyParser::parseRegister8(Parser& parser, GameboyRegisterValue& dest, int allowed)
{
	return parseRegisterTable(parser, dest, gameboyRegs8, std::size(gameboyRegs8), allowed);
}

bool GameboyParser::parseRegister16SP(Parser& parser, GameboyRegisterValue& dest, int allowed)
{
	return parseRegisterTable(parser, dest, gameboyRegs16SP, std::size(gameboyRegs16SP), allowed);
}

bool GameboyParser::parseRegister16AF(Parser& parser, GameboyRegisterValue& dest, int allowed)
{
	return parseRegisterTable(parser, dest, gameboyRegs16AF, std::size(gameboyRegs16AF), allowed);
}

bool GameboyParser::parseCondition(Parser& parser, GameboyRegisterValue& dest)
{
	return parseRegisterTable(parser, dest, gameboyConds, std::size(gameboyConds), GB_REG_BIT_ALL);
}

bool GameboyParser::parseHLIncDec(Parser& parser, GameboyRegisterValue& dest)
{
	CHECK(parser.matchToken(TokenType::LParen));

	// hli / hld
	if (!parseRegisterTable(parser, dest, gameboyHLIncDec16, std::size(gameboyHLIncDec16), GB_REG_BIT_ALL))
	{
		// hl+ / hl-
		CHECK(parseRegister16SP(parser, dest, GB_REG_BIT(GB_REG16_HL)));

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

bool GameboyParser::parseMemoryRegister16(Parser& parser, GameboyRegisterValue& dest, int allowed)
{
	CHECK(parser.matchToken(TokenType::LParen));
	CHECK(parseRegister16SP(parser, dest, allowed));
	CHECK(parser.matchToken(TokenType::RParen));

	return true;
}

bool GameboyParser::parseMemoryImmediate(Parser& parser, Expression& dest)
{
	CHECK(parser.matchToken(TokenType::LParen));
	dest = parser.parseExpression();
	CHECK(dest.isLoaded());
	CHECK(parser.matchToken(TokenType::RParen));

	return true;
}

bool GameboyParser::parseFF00PlusC(Parser& parser)
{
	CHECK(parser.matchToken(TokenType::LParen));

	const Token& token = parser.nextToken();
	CHECK(token.type == TokenType::Integer);
	CHECK(token.intValue == 0xFF00);

	CHECK(parser.matchToken(TokenType::Plus));

	GameboyRegisterValue tempReg;
	CHECK(parseRegister8(parser, tempReg, GB_REG_BIT(GB_REG8_C)));

	CHECK(parser.matchToken(TokenType::RParen));

	return true;
}

bool GameboyParser::parseSPImmediate(Parser& parser, Expression& dest, bool& isNegative)
{
	isNegative = false;

	GameboyRegisterValue tempReg;
	CHECK(parseRegister16SP(parser, tempReg, GB_REG_BIT(GB_REG16_SP)));

	const Token& token = parser.peekToken();
	if (token.type != TokenType::Plus && token.type != TokenType::Minus)
	{
		// Treat as +0
		dest = createConstExpression(0);
		return true;
	}
	parser.eatToken();
	isNegative = token.type == TokenType::Minus;

	dest = parser.parseExpression();
	CHECK(dest.isLoaded());

	return true;
}

bool GameboyParser::parseOpcodeParameter(Parser& parser, unsigned char paramType, GameboyRegisterValue& destReg, Expression& destImm, bool& isNegative)
{
	switch (paramType)
	{
	case GB_PARAM_REG8_MEMHL:
		if (parseRegister8(parser, destReg, GB_REG8_BIT_ALL))
		{
			return true;
		}
		if (parseMemoryRegister16(parser, destReg, GB_REG_BIT(GB_REG16_HL)))
		{
			destReg.num = GB_REG8_MEMHL;
			return true;
		}
		return false;
	case GB_PARAM_REG16_SP:
		return parseRegister16SP(parser, destReg, GB_REG16_BIT_ALL);
	case GB_PARAM_REG16_AF:
		return parseRegister16AF(parser, destReg, GB_REG16_BIT_ALL);
	case GB_PARAM_A:
		return parseRegister8(parser, destReg, GB_REG_BIT(GB_REG8_A));
	case GB_PARAM_MEMBC_MEMDE:
		return parseMemoryRegister16(parser, destReg, GB_REG_BIT(GB_REG16_BC) | GB_REG_BIT(GB_REG16_DE));
	case GB_PARAM_HL:
		return parseRegister16SP(parser, destReg, GB_REG_BIT(GB_REG16_HL));
	case GB_PARAM_MEMHL:
		return parseMemoryRegister16(parser, destReg, GB_REG_BIT(GB_REG16_HL));
	case GB_PARAM_HLI_HLD:
		return parseHLIncDec(parser, destReg);
	case GB_PARAM_SP:
		return parseRegister16SP(parser, destReg, GB_REG_BIT(GB_REG16_SP));
	case GB_PARAM_IMMEDIATE:
		destImm = parser.parseExpression();
		return destImm.isLoaded();
	case GB_PARAM_MEMIMMEDIATE:
		return parseMemoryImmediate(parser, destImm);
	case GB_PARAM_FF00_C:
		return parseFF00PlusC(parser);
	case GB_PARAM_SP_IMM:
		return parseSPImmediate(parser, destImm, isNegative);
	case GB_PARAM_CONDITION:
		return parseCondition(parser, destReg);
	default:
		return false;
	}
}

bool GameboyParser::parseOpcodeParameterList(Parser& parser, const tGameboyOpcode opcode, GameboyOpcodeVariables& vars)
{
	bool isNegative = false;
	if (opcode.lhs)
	{
		CHECK(parseOpcodeParameter(parser, opcode.lhs, vars.LeftParam, vars.ImmediateExpression, isNegative));
	}
	if (opcode.rhs)
	{
		CHECK(parser.matchToken(TokenType::Comma));
		CHECK(parseOpcodeParameter(parser, opcode.rhs, vars.RightParam, vars.ImmediateExpression, isNegative));
	}
	vars.IsNegative = isNegative;

	return true;
}

std::unique_ptr<CGameboyInstruction> GameboyParser::parseOpcode(Parser& parser)
{
	if (parser.peekToken().type != TokenType::Identifier)
		return nullptr;

	const Token& token = parser.nextToken();

	GameboyOpcodeVariables vars;
	bool paramFail = false;

	const std::wstring stringValue = token.getStringValue();
	for (int z = 0; GameboyOpcodes[z].name != nullptr; z++)
	{
		if (stringValue == GameboyOpcodes[z].name)
		{
			TokenizerPosition tokenPos = parser.getTokenizer()->getPosition();

			if (parseOpcodeParameterList(parser, GameboyOpcodes[z], vars))
			{
				// success, return opcode
				return std::make_unique<CGameboyInstruction>(GameboyOpcodes[z], vars);
			}

			parser.getTokenizer()->setPosition(tokenPos);
			paramFail = true;
		}
	}

	if (paramFail)
		parser.printError(token, L"Gameboy parameter failure in %S", stringValue);
	else
		parser.printError(token, L"Invalid Gameboy opcode: %S", stringValue);

	return nullptr;
}
