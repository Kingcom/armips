#include "Archs/ARM/ArmParser.h"

#include "Archs/ARM/Arm.h"
#include "Archs/ARM/CArmInstruction.h"
#include "Archs/ARM/CThumbInstruction.h"
#include "Commands/CDirectiveFile.h"
#include "Commands/CommandSequence.h"
#include "Core/Common.h"
#include "Parser/DirectivesParser.h"
#include "Parser/ExpressionParser.h"
#include "Parser/Parser.h"
#include "Util/Util.h"

#define CHECK(exp) if (!(exp)) return false;

const ArmRegisterDescriptor armRegisters[] = {
	{ "r0",  0 },  { "r1",  1 },  { "r2",  2 },  { "r3",  3 },
	{ "r4",  4 },  { "r5",  5 },  { "r6",  6 },  { "r7",  7 },
	{ "r8",  8 },  { "r9",  9 },  { "r10", 10 }, { "r11", 11 },
	{ "r12", 12 }, { "r13", 13 }, { "sp",  13 }, { "r14", 14 },
	{ "lr",  14 }, { "r15", 15 }, { "pc",  15 },
};

const ArmRegisterDescriptor armCopRegisters[] = {
	{ "c0",  0 },  { "c1",  1 },  { "c2",  2 },  { "c3",  3 },
	{ "c4",  4 },  { "c5",  5 },  { "c6",  6 },  { "c7",  7 },
	{ "c8",  8 },  { "c9",  9 },  { "c10", 10 }, { "c11", 11 },
	{ "c12", 12 }, { "c13", 13 }, { "c14", 14 }, { "c15", 15 },
};

const ArmRegisterDescriptor armCopNumbers[] = {
	{ "p0",  0 },  { "p1",  1 },  { "p2",  2 },  { "p3",  3 },
	{ "p4",  4 },  { "p5",  5 },  { "p6",  6 },  { "p7",  7 },
	{ "p8",  8 },  { "p9",  9 },  { "p10", 10 }, { "p11", 11 },
	{ "p12", 12 }, { "p13", 13 }, { "p14", 14 }, { "p15", 15 },
};

std::unique_ptr<CAssemblerCommand> parseDirectiveThumb(Parser& parser, int flags)
{
	Arm.SetThumbMode(true);
	return std::make_unique<ArmStateCommand>(false);
}

std::unique_ptr<CAssemblerCommand> parseDirectiveArm(Parser& parser, int flags)
{
	Arm.SetThumbMode(false);
	return std::make_unique<ArmStateCommand>(true);
}

std::unique_ptr<CAssemblerCommand> parseDirectivePool(Parser& parser, int flags)
{
	auto seq = std::make_unique<CommandSequence>();
	seq->addCommand(std::make_unique<CDirectiveAlignFill>(4,CDirectiveAlignFill::AlignVirtual));
	seq->addCommand(std::make_unique<ArmPoolCommand>());

	return seq;
}

const char* msgTemplate = R"(
	mov    r12,r12
	b      %after%
	.byte  0x64,0x64,0x00,0x00
	.if strlen(%text%) > 120
		.error "Maximum length of No$gba debug message is 120 bytes"
	.elseif strlen(%text%) > 0
		.ascii %text%
	.else
		.byte  0x00
	.endif
	.align %alignment%,0x00
	%after%:
)";

std::unique_ptr<CAssemblerCommand> parseDirectiveMsg(Parser& parser, int flags)
{
	Expression text = parser.parseExpression();
	if (!text.isLoaded())
		return nullptr;

	return parser.parseTemplate(msgTemplate, {
		{ "%after%", Global.symbolTable.getUniqueLabelName(true).string() },
		{ "%text%", text.toString() },
		{ "%alignment%", Arm.GetThumbMode() ? "2" : "4" }
	});
}

const DirectiveMap armDirectives = {
	{ ".thumb",	{ &parseDirectiveThumb,	0 } },
	{ ".arm",		{ &parseDirectiveArm,	0 } },
	{ ".pool",		{ &parseDirectivePool,	0 } },
	{ ".msg",		{ &parseDirectiveMsg,	0 } },
};

std::unique_ptr<CAssemblerCommand> ArmParser::parseDirective(Parser& parser)
{
	return parser.parseDirective(armDirectives);
}

bool ArmParser::parseRegisterTable(Parser& parser, ArmRegisterValue& dest, const ArmRegisterDescriptor* table, size_t count)
{
	const Token& token = parser.peekToken();
	if (token.type != TokenType::Identifier)
		return false;

	const Identifier &identifier = token.identifierValue();
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

bool ArmParser::parseRegister(Parser& parser, ArmRegisterValue& dest, int max)
{
	if (!parseRegisterTable(parser,dest,armRegisters, std::size(armRegisters)))
		return false;

	return dest.num <= max;
}

bool ArmParser::parseCopRegister(Parser& parser, ArmRegisterValue& dest)
{
	return parseRegisterTable(parser,dest,armCopRegisters, std::size(armCopRegisters));
}

bool ArmParser::parseCopNumber(Parser& parser, ArmRegisterValue& dest)
{
	return parseRegisterTable(parser,dest,armCopNumbers, std::size(armCopNumbers));
}

bool ArmParser::parseRegisterList(Parser& parser, int& dest, int validMask)
{
	ArmRegisterValue reg, reg2;

	dest = 0;
	while (true)
	{
		if (!parseRegister(parser,reg))
			return false;

		if (parser.peekToken().type == TokenType::Minus)
		{
			parser.eatToken();

			if (!parseRegister(parser,reg2) || reg2.num < reg.num)
				return false;
			
			for (int i = reg.num; i <= reg2.num; i++)
			{
				dest |= (1 << i);
			}
		} else {
			dest |= (1 << reg.num);
		}

		if (parser.peekToken().type != TokenType::Comma)
			break;

		parser.eatToken();
	}

	return (validMask & dest) == dest;
}

bool ArmParser::parseImmediate(Parser& parser, Expression& dest)
{
	TokenizerPosition pos = parser.getTokenizer()->getPosition();

	// check if it really is an immediate
	ArmOpcodeVariables tempVars;
	if (parsePsrTransfer(parser,tempVars,false))
		return false;

	parser.getTokenizer()->setPosition(pos);
	if (parseRegister(parser,tempVars.rd))
		return false;
	
	parser.getTokenizer()->setPosition(pos);
	if (parseCopNumber(parser,tempVars.rd))
		return false;
	
	parser.getTokenizer()->setPosition(pos);
	if (parseCopRegister(parser,tempVars.rd))
		return false;
	
	parser.getTokenizer()->setPosition(pos);
	dest = parser.parseExpression();
	return dest.isLoaded();
}

bool ArmParser::matchSymbol(Parser& parser, char symbol, bool optional)
{
	switch (symbol)
	{
	case '[':
		return parser.matchToken(TokenType::LBrack,optional);
	case ']':
		return parser.matchToken(TokenType::RBrack,optional);
	case ',':
		return parser.matchToken(TokenType::Comma,optional);
	case '!':
		return parser.matchToken(TokenType::Exclamation,optional);
	case '{':
		return parser.matchToken(TokenType::LBrace,optional);
	case '}':
		return parser.matchToken(TokenType::RBrace,optional);
	case '#':
		return parser.matchToken(TokenType::Hash,optional);
	case '=':
		return parser.matchToken(TokenType::Assign,optional);
	case '+':
		return parser.matchToken(TokenType::Plus,optional);
	}

	return false;
}

inline bool isNumber(char value)
{
	return (value >= '0' && value <= '9');
}

bool ArmParser::parseShift(Parser& parser, ArmOpcodeVariables& vars, bool immediateOnly)
{
	// no shift is also valid
	vars.Shift.UseShift = false;
	if (parser.peekToken().type != TokenType::Comma)
		return true;

	parser.eatToken();

	// load shift mode
	const Token& shiftMode = parser.nextToken();
	if (shiftMode.type != TokenType::Identifier)
		return false;

	std::string stringValue = shiftMode.identifierValue().string();
	
	bool hasNumber = isNumber(stringValue.back());
	int64_t number = 0;

	// handle modeXX syntax
	if (hasNumber)
	{
		int64_t multiplier = 1;
		while (isNumber(stringValue.back()))
		{
			number += multiplier*(stringValue.back() - '0');
			multiplier *= 10;
			stringValue.pop_back();
		}
	}

	if (stringValue == "lsl" || stringValue == "asl")
		vars.Shift.Type = 0;
	else if (stringValue == "lsr")
		vars.Shift.Type = 1;
	else if (stringValue == "asr")
		vars.Shift.Type = 2;
	else if (stringValue == "ror")
		vars.Shift.Type = 3;
	else if (stringValue == "rrx")
		vars.Shift.Type = 4;
	else 
		return false;

	if (hasNumber)
	{
		vars.Shift.ShiftExpression = createConstExpression(number);
		vars.Shift.ShiftByRegister = false;
	} else if (parseRegister(parser,vars.Shift.reg))
	{
		if (immediateOnly)
			return false;

		vars.Shift.ShiftByRegister = true;
	} else {
		if (parser.peekToken().type == TokenType::Hash)
			parser.eatToken();
		
		if (!parseImmediate(parser,vars.Shift.ShiftExpression))
			return false;

		vars.Shift.ShiftByRegister = false;
	}
	
	vars.Shift.UseShift = true;
	return true;
}

bool ArmParser::parsePseudoShift(Parser& parser, ArmOpcodeVariables& vars, int type)
{
	vars.Shift.Type = type;

	if (parseRegister(parser,vars.Shift.reg))
	{
		vars.Shift.ShiftByRegister = true;
	} else {
		if (parser.peekToken().type == TokenType::Hash)
			parser.eatToken();
		
		if (!parseImmediate(parser,vars.Shift.ShiftExpression))
			return false;

		vars.Shift.ShiftByRegister = false;
	}
	
	vars.Shift.UseShift = true;
	return true;
}

int ArmParser::decodeCondition(const std::string& text, size_t& pos)
{
	if (pos+2 <= text.size())
	{
		char c1 = text[pos+0];
		char c2 = text[pos+1];
		pos += 2;

		if (c1 == 'e' && c2 == 'q') return 0;
		if (c1 == 'n' && c2 == 'e') return 1;
		if (c1 == 'c' && c2 == 's') return 2;
		if (c1 == 'h' && c2 == 's') return 2;
		if (c1 == 'c' && c2 == 'c') return 3;
		if (c1 == 'l' && c2 == 'o') return 3;
		if (c1 == 'm' && c2 == 'i') return 4;
		if (c1 == 'p' && c2 == 'l') return 5;
		if (c1 == 'v' && c2 == 's') return 6;
		if (c1 == 'v' && c2 == 'c') return 7;
		if (c1 == 'h' && c2 == 'i') return 8;
		if (c1 == 'l' && c2 == 's') return 9;
		if (c1 == 'g' && c2 == 'e') return 10;
		if (c1 == 'l' && c2 == 't') return 11;
		if (c1 == 'g' && c2 == 't') return 12;
		if (c1 == 'l' && c2 == 'e') return 13;
		if (c1 == 'a' && c2 == 'l') return 14;

		pos -= 2;
	}

	return 14;
}

bool ArmParser::decodeAddressingMode(const std::string& text, size_t& pos, unsigned char& dest)
{
	if (pos+2 > text.size())
		return false;

	char c1 = text[pos+0];
	char c2 = text[pos+1];

	if      (c1 == 'i' && c2 == 'b') dest = ARM_AMODE_IB;
	else if (c1 == 'i' && c2 == 'a') dest = ARM_AMODE_IA;
	else if (c1 == 'd' && c2 == 'b') dest = ARM_AMODE_DB;
	else if (c1 == 'd' && c2 == 'a') dest = ARM_AMODE_DA;
	else if (c1 == 'e' && c2 == 'd') dest = ARM_AMODE_ED;
	else if (c1 == 'f' && c2 == 'd') dest = ARM_AMODE_FD;
	else if (c1 == 'e' && c2 == 'a') dest = ARM_AMODE_EA;
	else if (c1 == 'f' && c2 == 'a') dest = ARM_AMODE_FA;
	else
		return false;

	pos += 2;
	return true;
}

bool ArmParser::decodeXY(const std::string& text, size_t& pos, bool& dest)
{
	if (pos >= text.size())
		return false;

	if (text[pos] == 't')
		dest = true;
	else if (text[pos] == 'b')
		dest = false;
	else
		return false;

	pos++;
	return true;
}

void ArmParser::decodeS(const std::string& text, size_t& pos, bool& dest)
{
	dest = pos < text.size() && text[pos] == 's';
	if (dest)
		pos++;
}

bool ArmParser::decodeArmOpcode(const std::string& name, const tArmOpcode& opcode, ArmOpcodeVariables& vars)
{
	vars.Opcode.c = vars.Opcode.a = 0;
	vars.Opcode.s = false;

	const char* encoding = opcode.name;
	size_t pos = 0;

	while (*encoding != 0)
	{
		switch (*encoding++)
		{
		case 'C':	// condition
			vars.Opcode.c = decodeCondition(name,pos);
			break;
		case 'S':	// set flag
			decodeS(name,pos,vars.Opcode.s);
			break;
		case 'A':	// addressing mode
			CHECK(decodeAddressingMode(name,pos,vars.Opcode.a));
			break;
		case 'X':	// x flag
			CHECK(decodeXY(name,pos,vars.Opcode.x));
			break;
		case 'Y':	// y flag
			CHECK(decodeXY(name,pos,vars.Opcode.y));
			break;
		default:
			CHECK(pos < name.size());
			CHECK(*(encoding-1) == name[pos++]);
			break;
		}
	}

	return pos >= name.size();
}

void ArmParser::parseWriteback(Parser& parser, bool& dest)
{
	dest = parser.peekToken().type == TokenType::Exclamation;
	if (dest)
		parser.eatToken();
}

void ArmParser::parsePsr(Parser& parser, bool& dest)
{
	dest = parser.peekToken().type == TokenType::Caret;
	if (dest)
		parser.eatToken();
}

void ArmParser::parseSign(Parser& parser, bool& dest)
{
	switch (parser.peekToken().type)
	{
	case TokenType::Plus:
		dest = true;
		parser.eatToken();
		break;
	case TokenType::Minus:
		dest = false;
		parser.eatToken();
		break;
	default:
		dest = true;
		break;
	}
}

bool ArmParser::parsePsrTransfer(Parser& parser, ArmOpcodeVariables& vars, bool shortVersion)
{
	const Token& token = parser.nextToken();
	if (token.type != TokenType::Identifier)
		return false;

	const std::string &stringValue = token.identifierValue().string();
	size_t pos = 0;
	if (startsWith(stringValue,"cpsr"))
	{
		vars.PsrData.spsr = false;
		pos = 4;
	} else if (startsWith(stringValue,"spsr"))
	{
		vars.PsrData.spsr = true;
		pos = 4;
	} else {
		return false;
	}

	if (shortVersion)
		return pos == stringValue.size();

	if (pos == stringValue.size())
	{
		vars.PsrData.field = 0xF;
		return true;
	}

	if (stringValue[pos++] != '_')
		return false;

	if (startsWith(stringValue,"ctl",pos))
	{
		vars.PsrData.field = 1;
		return pos+3 == stringValue.size();
	} 
	
	if (startsWith(stringValue,"flg",pos))
	{
		vars.PsrData.field = 8;
		return pos+3 == stringValue.size();
	}
	
	vars.PsrData.field = 0;
	for (int i = 0; i < 4; i++)
	{
		if (pos == stringValue.size())
			break;

		switch(stringValue[pos++])
		{
		case 'f':
			if (vars.PsrData.field & 8)
				return false;	// can only appear once
			vars.PsrData.field |= 8;
			break;
		case 's':
			if (vars.PsrData.field & 4)
				return false;	// can only appear once
			vars.PsrData.field |= 4;
			break;
		case 'x':
			if (vars.PsrData.field & 2)
				return false;	// can only appear once
			vars.PsrData.field |= 2;
			break;
		case 'c':
			if (vars.PsrData.field & 1)
				return false;	// can only appear once
			vars.PsrData.field |= 1;
			break;
		default:
			return false;	// has to be one of those
		}
	}

	return true;
}

bool ArmParser::parseArmParameters(Parser& parser, const tArmOpcode& opcode, ArmOpcodeVariables& vars)
{
	const char* encoding = opcode.mask;

	ArmRegisterValue tempRegister;
	
	vars.Shift.UseShift = false;
	vars.Shift.UseFinal = false;
	vars.psr = false;
	vars.writeback = false;
	vars.SignPlus = false;
	vars.Opcode.UseNewEncoding = false;
	vars.Opcode.UseNewType = false;

	while (*encoding != 0)
	{
		bool optional = *encoding == '/';
		if (optional)
			encoding++;

		switch (*encoding++)
		{
		case 'd': // register
			CHECK(parseRegister(parser,vars.rd,*encoding++ == '1' ? 14 : 15));
			break;
		case 's': // register
			CHECK(parseRegister(parser,vars.rs,*encoding++ == '1' ? 14 : 15));
			break;
		case 'n': // register
			CHECK(parseRegister(parser,vars.rn,*encoding++ == '1' ? 14 : 15));
			break;
		case 'm': // register
			CHECK(parseRegister(parser,vars.rm,*encoding++ == '1' ? 14 : 15));
			break;
		case 'D': // cop register
			CHECK(parseCopRegister(parser,vars.CopData.cd));
			break;
		case 'N': // cop register
			CHECK(parseCopRegister(parser,vars.CopData.cn));
			break;
		case 'M': // cop register
			CHECK(parseCopRegister(parser,vars.CopData.cm));
			break;
		case 'W':	// writeback
			parseWriteback(parser,vars.writeback);
			break;
		case 'p':	// psr
			parsePsr(parser,vars.psr);
			break;
		case 'P':	// msr/mrs psr data
			CHECK(parsePsrTransfer(parser,vars,*encoding++ == '1'));
			break;
		case 'R':	// register list
			CHECK(parseRegisterList(parser,vars.rlist,0xFFFF));
			break;
		case 'S':	// shift
			CHECK(parseShift(parser,vars,*encoding++ == '1'));
			break;
		case 'I':	// immediate
		case 'i':
			CHECK(parseImmediate(parser,vars.ImmediateExpression));
			vars.ImmediateBitLen = 32;
			break;
		case 'j':	// variable bit immediate
			CHECK(parseImmediate(parser,vars.ImmediateExpression));
			vars.ImmediateBitLen = *encoding++;
			break;
		case 'X': // cop number
			CHECK(parseCopNumber(parser,vars.CopData.pn));
			break;
		case 'Y':	// cop opcode number
			CHECK(parseImmediate(parser,vars.CopData.CpopExpression));
			vars.ImmediateBitLen = 4;
			break;
		case 'Z':	// cop info number
			CHECK(parseImmediate(parser,vars.CopData.CpinfExpression));
			vars.ImmediateBitLen = 3;
			break;
		case 'z':	// shift for pseudo opcodes
			CHECK(parsePseudoShift(parser,vars,*encoding++));
			break;
		case 'v':	// sign for register index parameter
			parseSign(parser,vars.SignPlus);
			break;
		default:
			CHECK(matchSymbol(parser,*(encoding-1),optional));
			break;
		}
	}

	// the next token has to be a separator, else the parameters aren't
	// completely parsed
	return parser.nextToken().type == TokenType::Separator;
}

std::unique_ptr<CArmInstruction> ArmParser::parseArmOpcode(Parser& parser)
{
	if (parser.peekToken().type != TokenType::Identifier)
		return nullptr;

	const Token &token = parser.nextToken();

	ArmOpcodeVariables vars;
	bool paramFail = false;

	const Identifier &identifier = token.identifierValue();
	for (int z = 0; ArmOpcodes[z].name != nullptr; z++)
	{
		if ((ArmOpcodes[z].flags & ARM_ARM9) && Arm.getVersion() == AARCH_GBA)
			continue;

		if (decodeArmOpcode(identifier.string(),ArmOpcodes[z],vars))
		{
			TokenizerPosition tokenPos = parser.getTokenizer()->getPosition();

			if (parseArmParameters(parser,ArmOpcodes[z],vars))
			{
				// success, return opcode
				return std::make_unique<CArmInstruction>(ArmOpcodes[z],vars);
			}

			parser.getTokenizer()->setPosition(tokenPos);
			paramFail = true;
		}
	}

	if (paramFail)
		parser.printError(token, "ARM parameter failure");
	else
		parser.printError(token, "Invalid ARM opcode");

	return nullptr;
}

bool ArmParser::parseThumbParameters(Parser& parser, const tThumbOpcode& opcode, ThumbOpcodeVariables& vars)
{
	const char* encoding = opcode.mask;

	ArmRegisterValue tempRegister;
	int value;

	while (*encoding != 0)
	{
		bool optional = *encoding == '/';
		if (optional)
			encoding++;

		switch (*encoding++)
		{
		case 'd': // register
			CHECK(parseRegister(parser,vars.rd,7));
			break;
		case 's': // register
			CHECK(parseRegister(parser,vars.rs,7));
			break;
		case 'n': // register
			CHECK(parseRegister(parser,vars.rn,7));
			break;
		case 'o': // register
			CHECK(parseRegister(parser,vars.ro,7));
			break;
		case 'D': // register
			CHECK(parseRegister(parser,vars.rd,15));
			break;
		case 'S': // register
			CHECK(parseRegister(parser,vars.rs,15));
			break;
		case 'r': // forced register
			CHECK(parseRegister(parser,tempRegister,15));
			CHECK(*encoding++ == tempRegister.num);
			break;
		case 'R':	// register list
			value = encoding[0] | (encoding[1] << 8);
			CHECK(parseRegisterList(parser,vars.rlist,value));
			encoding += 2;
			break;
		case 'I':	// immediate
		case 'i':
			CHECK(parseImmediate(parser,vars.ImmediateExpression));
			vars.ImmediateBitLen = *encoding++;
			break;
		default:
			CHECK(matchSymbol(parser,*(encoding-1),optional));
			break;
		}
	}
	
	// the next token has to be a separator, else the parameters aren't
	// completely parsed
	return parser.nextToken().type == TokenType::Separator;
}

std::unique_ptr<CThumbInstruction> ArmParser::parseThumbOpcode(Parser& parser)
{
	if (parser.peekToken().type != TokenType::Identifier)
		return nullptr;

	const Token &token = parser.nextToken();

	ThumbOpcodeVariables vars;
	bool paramFail = false;

	const Identifier &identifier = token.identifierValue();
	for (int z = 0; ThumbOpcodes[z].name != nullptr; z++)
	{
		if ((ThumbOpcodes[z].flags & THUMB_ARM9) && Arm.getVersion() == AARCH_GBA)
			continue;

		if (identifier == ThumbOpcodes[z].name)
		{
			TokenizerPosition tokenPos = parser.getTokenizer()->getPosition();
			
			if (parseThumbParameters(parser,ThumbOpcodes[z],vars))
			{
				// success, return opcode
				return std::make_unique<CThumbInstruction>(ThumbOpcodes[z],vars);
			}

			parser.getTokenizer()->setPosition(tokenPos);
			paramFail = true;
		}
	}

	if (paramFail)
		parser.printError(token, "THUMB parameter failure in %S",identifier);
	else
		parser.printError(token, "Invalid THUMB opcode: %S",identifier);
	
	return nullptr;
}
