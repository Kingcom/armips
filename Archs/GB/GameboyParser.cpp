#include "Archs/GB/CGameboyInstruction.h"
#include "Archs/GB/Gameboy.h"
#include "Archs/GB/GameboyOpcodes.h"
#include "Archs/GB/GameboyParser.h"
#include "Core/Expression.h"
#include "Parser/DirectivesParser.h"
#include "Parser/Tokenizer.h"

#define CHECK(exp) if (!(exp)) return false;

const GameboyParameterDescriptor gameboyRegs8[] = {
	{ L"b", GB_REG8_B }, { L"c", GB_REG8_C },
	{ L"d", GB_REG8_D }, { L"e", GB_REG8_E },
	{ L"h", GB_REG8_H }, { L"l", GB_REG8_L },
	{ L"a", GB_REG8_A },
};

const GameboyParameterDescriptor gameboyRegs16[] = {
	{ L"bc", GB_REG16_BC }, { L"de", GB_REG16_DE },
	{ L"hl", GB_REG16_HL }, { L"sp", GB_REG16_SP },
};

const DirectiveMap gameboyDirectives = { };

std::unique_ptr<CAssemblerCommand> GameboyParser::parseDirective(Parser& parser)
{
	return parser.parseDirective(gameboyDirectives);
}

bool GameboyParser::parseRegisterTable(Parser& parser, GameboyRegisterValue& dest, const GameboyParameterDescriptor* table, size_t count, int allowed)
{
	const Token& token = parser.peekToken();

	if (token.type != TokenType::Identifier)
		return false;

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

bool GameboyParser::parseRegister16(Parser& parser, GameboyRegisterValue& dest, int allowed)
{
	return parseRegisterTable(parser, dest, gameboyRegs16, std::size(gameboyRegs16), allowed);
}

bool GameboyParser::parseMemoryRegister16(Parser& parser, GameboyRegisterValue& dest, int allowed)
{
	CHECK(parser.matchToken(TokenType::LParen));
	CHECK(parseRegister16(parser, dest, allowed));
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

bool GameboyParser::parseOpcodeParameter(Parser& parser, unsigned char paramType, GameboyRegisterValue& destReg, Expression& destImm)
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
	case GB_PARAM_REG16:
		return parseRegister16(parser, destReg, GB_REG16_BIT_ALL);
	case GB_PARAM_A:
		return parseRegister8(parser, destReg, GB_REG_BIT(GB_REG8_A));
	case GB_PARAM_MEMBC_MEMDE:
		return parseMemoryRegister16(parser, destReg,
			GB_REG_BIT(GB_REG16_BC) | GB_REG_BIT(GB_REG16_DE));
	case GB_PARAM_SP:
		return parseRegister16(parser, destReg, GB_REG_BIT(GB_REG16_SP));
	case GB_PARAM_IMMEDIATE:
		destImm = parser.parseExpression();
		return destImm.isLoaded();
	case GB_PARAM_MEMIMMEDIATE:
		return parseMemoryImmediate(parser, destImm);
	default:
		return false;
	}
}

bool GameboyParser::parseOpcodeParameterList(Parser& parser, const tGameboyOpcode opcode, GameboyOpcodeVariables& vars)
{
	if (opcode.lhs)
	{
		CHECK(parseOpcodeParameter(parser, opcode.lhs, vars.LeftParam, vars.ImmediateExpression));
	}
	if (opcode.rhs)
	{
		CHECK(parser.matchToken(TokenType::Comma));
		CHECK(parseOpcodeParameter(parser, opcode.rhs, vars.RightParam, vars.ImmediateExpression));
	}

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
