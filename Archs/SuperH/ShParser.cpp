#include "Archs/SuperH/ShParser.h"
#include "Archs/SuperH/SuperH.h"
#include "Commands/CDirectiveFile.h"
#include "Core/Common.h"
#include "Parser/DirectivesParser.h"
#include "Parser/ExpressionParser.h"
#include "Parser/Parser.h"
#include "Util/Util.h"

#define CHECK(exp) if (!(exp)) return false;

static const char *shSpecialForcedRegisters[] =
{
	"r0", "sr", "gbr", "vbr", "mach", "macl", "pr", "pc", nullptr
};

const ShRegisterDescriptor shRegisters[] = {
	{ "r0",  0 },   { "r1",  1},   { "r2",  2 },  { "r3",  3 },
	{ "r4",  4 },   { "r5",  5},   { "r6",  6 },  { "r7",  7 },
	{ "r8",  8 },   { "r9",  9},   { "r10", 10 }, { "r11", 11 },
	{ "r12", 12 },  { "r13", 13},  { "r14", 14 }, { "r15", 15 },
	{ "@r0",  0 },   { "@r1",  1},   { "@r2",  2 },  { "@r3",  3 },
	{ "@r4",  4 },   { "@r5",  5},   { "@r6",  6 },  { "@r7",  7 },
	{ "@r8",  8 },   { "@r9",  9},   { "@r10", 10 }, { "@r11", 11 },
	{ "@r12", 12 },  { "@r13", 13},  { "@r14", 14 }, { "@r15", 15 },
};

std::unique_ptr<CAssemblerCommand> parseDirectiveShImportObj(Parser& parser, int flags)
{
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
		
		return std::make_unique<DirectiveObjImport>(inputName.path(),ctorName);
	}

	return std::make_unique<DirectiveObjImport>(inputName.path());
}

const DirectiveMap shDirectives = {
	{ ".importobj",		{ &parseDirectiveShImportObj,		0 } },
	{ ".importlib",		{ &parseDirectiveShImportObj,		0 } },
};

std::unique_ptr<CAssemblerCommand> ShParser::parseDirective(Parser& parser)
{
	return parser.parseDirective(shDirectives);
}

bool ShParser::parseRegisterTable(Parser& parser, ShRegisterValue& dest, const ShRegisterDescriptor* table, size_t count)
{
	const Token &token = parser.peekToken(0);

	if (token.type != TokenType::Identifier)
		return false;

	const auto &identifier = token.identifierValue();
	for (size_t i = 0; i < count; i++)
	{
		if (identifier == table[i].name)
		{
			dest.name = identifier;
			dest.num = table[i].num;
			parser.eatToken();
			return true;
		}
	}

	return false;
}

bool ShParser::parseRegister(Parser& parser, ShRegisterValue& dest)
{
	dest.type = ShRegisterType::Normal;
	return parseRegisterTable(parser, dest, shRegisters, 32);
}

bool ShParser::parseImmediate(Parser& parser, Expression& dest)
{
	// check for (reg) or reg sequence
	TokenizerPosition pos = parser.getTokenizer()->getPosition();

	bool hasParen = parser.peekToken().type == TokenType::LParen;
	if (hasParen)
		parser.eatToken();

	ShRegisterValue tempValue;
	bool isRegister = parseRegister(parser,tempValue);
	parser.getTokenizer()->setPosition(pos);

	if (isRegister)
		return false;

	dest = parser.parseExpression();
	return dest.isLoaded();
}

bool ShParser::matchSymbol(Parser& parser, char symbol)
{
	switch (symbol)
	{
	case '(':
		return parser.matchToken(TokenType::LParen);
	case ')':
		return parser.matchToken(TokenType::RParen);
	case ',':
		return parser.matchToken(TokenType::Comma);
	case '#':
		return parser.matchToken(TokenType::Hash);
	case '-':
		return parser.matchToken(TokenType::Minus);
	case '+':
		return parser.matchToken(TokenType::Plus);
	}

	return false;
}

static bool decodeImmediateSize(const char*& encoding, ShImmediateType& dest)
{
	int num = 0;
	while (*encoding >= '0' && *encoding <= '9')
	{
		num = num*10 + *encoding-'0';
		encoding++;
	}

	switch (num)
	{
	case 4:
		dest = ShImmediateType::Immediate4;
		break;
	case 8:
		dest = ShImmediateType::Immediate8;
		break;
	case 12:
		dest = ShImmediateType::Immediate12;
		break;
	default:
		return false;
	}

	return true;
}

bool ShParser::decodeOpcode(Parser& parser, const tShOpcode& opcode)
{
	const char* encoding = opcode.name;
	size_t pos = 0;

	registers.reset();
	immediate.reset();
	opcodeData.reset();

	const Token &token = parser.nextToken();
	if (token.type != TokenType::Identifier)
		return false;
		
	const Identifier &identifier = token.identifierValue();
	std::string name = identifier.string();

	while (*encoding != 0)
	{
		switch (*encoding++)
		{
		case '/':
			CHECK(pos >= name.size());
			CHECK(parser.nextToken().type == TokenType::Div);
			CHECK(parser.peekToken().type == TokenType::Identifier);
			
			name = parser.nextToken().identifierValue().string();
			pos = 0;
			break;
		default:
			CHECK(pos < name.size());
			CHECK(*(encoding-1) == name[pos++]);
			break;
		}
	}

	return pos >= name.size();
}

bool ShParser::parseParameters(Parser& parser, const tShOpcode& opcode)
{
	const char* encoding = opcode.encoding;

	// initialize opcode variables
	immediate.primary.type = ShImmediateType::None;

	while (*encoding != 0)
	{
		// Some registers in instructions are forced
		// and do not have a numerical representation (except for r0.)
		// This handles said forced registers.
		if (opcode.flags & SH_FREG)
		{
			const char **fReg = shSpecialForcedRegisters;
			const Token &token = parser.peekToken();
			bool skip = false;
			while (*fReg)
			{
				size_t length = strlen(*fReg);
				if (memcmp(*fReg, encoding, length) == 0)
				{
					if (token.type != TokenType::Identifier)
						break;

					const auto &identifier = token.identifierValue();
					if (identifier.string() == std::string(*fReg))
					{
						skip = true;
						encoding += length;
						parser.eatToken();
						break;
					}
					break;
				}
				fReg += 1;
			}
			if (skip)
				continue;
		}

		switch (*encoding++)
		{
		case 't':	// register
			CHECK(parseRegister(parser,registers.grt));
			break;
		case 's':	// register
			CHECK(parseRegister(parser,registers.grs));
			break;
		case 'i':	// primary immediate
			CHECK(parseImmediate(parser,immediate.primary.expression));
			CHECK(decodeImmediateSize(encoding,immediate.primary.type));
			break;
		case '@':
			// '@' is not a separate token, and at the same time it's
			// part of the SuperH instruction parameter scheme,
			// so I came up with this rather ugly solution to handle it. 
			if (parser.peekToken().type != TokenType::Identifier)
				return false;
			if (parser.peekToken().identifierValue().string()[0] != '@')
				return false;
			if (*encoding == 't')
			{
				CHECK(parseRegister(parser,registers.grt));
			}
			else if (*encoding == 's')
			{
				CHECK(parseRegister(parser,registers.grs));
			}
			else if (*encoding == '(' || *encoding == '-') // "mov.* r0,@(i4,t)", "mov.* s,@-t" and the others
			{
				parser.eatToken();
				CHECK(matchSymbol(parser, *encoding));
			}
			else
			{
				return false;
			}
			encoding++;
			break;
		default:
			CHECK(matchSymbol(parser,*(encoding-1)));
			break;
		}
	}

	opcodeData.opcode = opcode;

	// the next token has to be a separator, else the parameters aren't
	// completely parsed

	return parser.nextToken().type == TokenType::Separator;

}

std::unique_ptr<CShInstruction> ShParser::parseOpcode(Parser& parser)
{
	if (parser.peekToken().type != TokenType::Identifier)
		return nullptr;

	const ShArchDefinition& arch = shArchs[SuperH.getVersion()];
	const Token &token = parser.peekToken();

	bool paramFail = false;
	for (int z = 0; shOpcodes[z].name != nullptr; z++)
	{
		if ((shOpcodes[z].archs & arch.supportSets) == 0)
			continue;
		if ((shOpcodes[z].archs & arch.excludeMask) != 0)
			continue;

		TokenizerPosition tokenOpcodePos = parser.getTokenizer()->getPosition();
		if (decodeOpcode(parser, shOpcodes[z]))
		{
			TokenizerPosition tokenPos = parser.getTokenizer()->getPosition();

			if (parseParameters(parser,shOpcodes[z]))
			{
				// success, return opcode
				return std::make_unique<CShInstruction>(opcodeData,immediate,registers);
			}

			parser.getTokenizer()->setPosition(tokenPos);
			paramFail = true;
		}
		parser.getTokenizer()->setPosition(tokenOpcodePos);
	}

	if (paramFail)
		parser.printError(token, "SuperH parameter failure");
	else
		parser.printError(token, "Invalid SuperH opcode");

	return nullptr;
}

void ShOpcodeFormatter::handleOpcodeName(const ShOpcodeData& opData)
{
	const char* encoding = opData.opcode.name;

	while (*encoding++ != 0)
	{
		buffer += *(encoding-1);
	}
}

void ShOpcodeFormatter::handleImmediate(ShImmediateType type, unsigned int originalValue, unsigned int opcodeFlags)
{
	buffer += tfm::format("0x%X", originalValue);
}

void ShOpcodeFormatter::handleOpcodeParameters(const ShOpcodeData& opData, const ShRegisterData& regData,
	const ShImmediateData& immData)
{
	const char* encoding = opData.opcode.encoding;

	ShImmediateType type;
	while (*encoding != 0)
	{
		switch (*encoding++)
		{
		case 's':	// register
			if (*encoding == 'r')
			{
				buffer += "sr";
				encoding += 1;
				break;
			}
			buffer += regData.grs.name.string();
			break;
		case 't':	// register
			buffer += regData.grt.name.string();
			break;
		case 'i':	// primary immediate
			decodeImmediateSize(encoding,type);
			handleImmediate(immData.primary.type,immData.primary.originalValue,opData.opcode.flags);
			break;
		default:
			buffer += *(encoding-1);
			break;
		}
	}
}

const std::string& ShOpcodeFormatter::formatOpcode(const ShOpcodeData& opData, const ShRegisterData& regData,
	const ShImmediateData& immData)
{
	buffer = "   ";
	handleOpcodeName(opData);

	while (buffer.size() < 11)
		buffer += ' ';

	handleOpcodeParameters(opData,regData,immData);
	return buffer;
}
