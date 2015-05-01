#include "stdafx.h"
#include "ArmParser.h"
#include "Parser/Parser.h"
#include "Parser/ExpressionParser.h"
#include "Parser/DirectivesParser.h"
#include "Util/Util.h"
#include "Core/Common.h"
#include "Commands/CDirectiveFile.h"

#define CHECK(exp) if (!(exp)) return false;

const ArmRegisterDescriptor armRegisters[] = {
	{ L"r0", 0 },	{ L"r1", 1 },	{ L"r2", 2 },	{ L"r3", 3 },
	{ L"r4", 4 },	{ L"r5", 5 },	{ L"r6", 6 },	{ L"r7", 7 },
	{ L"r8", 8 },	{ L"r9", 9 },	{ L"r10", 10 },	{ L"r11", 11 },
	{ L"r12", 12 },	{ L"r13", 13 },	{ L"sp", 13 },	{ L"r14", 14 },
	{ L"lr", 14 },	{ L"r15", 15 }, { L"pc", 15 },
};

const ArmRegisterDescriptor armCopRegisters[] = {
	{ L"c0", 0 },	{ L"c1", 1 },	{ L"c2", 2 },	{ L"c3", 3 },
	{ L"c4", 4 },	{ L"c5", 5 },	{ L"c6", 6 },	{ L"c7", 7 },
	{ L"c8", 8 },	{ L"c9", 9 },	{ L"c10", 10 },	{ L"c11", 11 },
	{ L"c12", 12 },	{ L"c13", 13 },	{ L"c14", 14 },	{ L"c15", 15 },
};

const ArmRegisterDescriptor armCopNumbers[] = {
	{ L"p0", 0 },	{ L"p1", 1 },	{ L"p2", 2 },	{ L"p3", 3 },
	{ L"p4", 4 },	{ L"p5", 5 },	{ L"p6", 6 },	{ L"p7", 7 },
	{ L"p8", 8 },	{ L"p9", 9 },	{ L"p10", 10 },	{ L"p11", 11 },
	{ L"p12", 12 },	{ L"p13", 13 },	{ L"p14", 14 },	{ L"p15", 15 },
};

CAssemblerCommand* parseDirectiveThumb(Tokenizer& tokenizer, int flags)
{
	Arm.SetThumbMode(true);
	return new ArmStateCommand(false);
}

CAssemblerCommand* parseDirectiveArm(Tokenizer& tokenizer, int flags)
{
	Arm.SetThumbMode(false);
	return new ArmStateCommand(true);
}

CAssemblerCommand* parseDirectivePool(Tokenizer& tokenizer, int flags)
{
	CommandSequence* seq = new CommandSequence();
	seq->addCommand(new CDirectiveAlignFill(4,CDirectiveAlignFill::Align));
	seq->addCommand(new ArmPoolCommand());

	return seq;
}

const wchar_t* msgTemplate =
	L"mov    r12,r12\n"
	L"b      %after%\n"
	L".byte  0x64,0x64,0x00,0x00\n"
	L".ascii %text%\n"
	L".align %alignment%\n"
	L"%after%:"
;

CAssemblerCommand* parseDirectiveMsg(Tokenizer& tokenizer, int flags)
{
	Expression text = parseExpression(tokenizer);
	if (text.isLoaded() == false)
		return nullptr;

	return parseTemplate(msgTemplate, {
		{ L"%after%", Global.symbolTable.getUniqueLabelName() },
		{ L"%text%", text.toString() },
		{ L"%alignment%", Arm.GetThumbMode() == true ? L"2" : L"4" }
	});
}

const DirectiveEntry armDirectives[] = {
	{ L".thumb",	&parseDirectiveThumb,	0 },
	{ L".arm",		&parseDirectiveArm,		0 },
	{ L".pool",		&parseDirectivePool,	0 },
	{ L".msg",		&parseDirectiveMsg,		0 },
	{ NULL,			NULL,					0 }
};

CAssemblerCommand* ArmParser::parseDirective(Tokenizer& tokenizer)
{
	return ::parseDirective(tokenizer,armDirectives);
}

bool ArmParser::parseRegisterTable(Tokenizer& tokenizer, ArmRegisterValue& dest, const ArmRegisterDescriptor* table, size_t count)
{
	Token& token = tokenizer.peekToken();
	if (token.type != TokenType::Identifier)
		return false;

	for (size_t i = 0; i < count; i++)
	{
		if (token.stringValue == table[i].name)
		{
			dest.name = token.stringValue;
			dest.num = table[i].num;
			tokenizer.eatToken();
			return true;
		}
	}

	return false;
}

bool ArmParser::parseRegister(Tokenizer& tokenizer, ArmRegisterValue& dest, int max)
{
	if (parseRegisterTable(tokenizer,dest,armRegisters,ARRAY_SIZE(armRegisters)) == false)
		return false;

	return dest.num <= max;
}

bool ArmParser::parseCopRegister(Tokenizer& tokenizer, ArmRegisterValue& dest)
{
	return parseRegisterTable(tokenizer,dest,armCopRegisters,ARRAY_SIZE(armCopRegisters));
}

bool ArmParser::parseCopNumber(Tokenizer& tokenizer, ArmRegisterValue& dest)
{
	return parseRegisterTable(tokenizer,dest,armCopNumbers,ARRAY_SIZE(armCopNumbers));
}

bool ArmParser::parseRegisterList(Tokenizer& tokenizer, int& dest, int validMask)
{
	ArmRegisterValue reg, reg2;

	dest = 0;
	while (true)
	{
		if (parseRegister(tokenizer,reg) == false)
			return false;

		if (tokenizer.peekToken().type == TokenType::Minus)
		{
			tokenizer.eatToken();

			if (parseRegister(tokenizer,reg2) == false || reg2.num < reg.num)
				return false;
			
			for (int i = reg.num; i <= reg2.num; i++)
			{
				dest |= (1 << i);
			}
		} else {
			dest |= (1 << reg.num);
		}

		if (tokenizer.peekToken().type != TokenType::Comma)
			break;

		tokenizer.eatToken();
	}

	return (validMask & dest) == dest;
}

bool ArmParser::parseImmediate(Tokenizer& tokenizer, Expression& dest)
{
	dest = parseExpression(tokenizer);
	return dest.isLoaded();
}

bool ArmParser::matchSymbol(Tokenizer& tokenizer, wchar_t symbol, bool optional)
{
	switch (symbol)
	{
	case '[':
		return matchToken(tokenizer,TokenType::LBrack,optional);
	case ']':
		return matchToken(tokenizer,TokenType::RBrack,optional);
	case ',':
		return matchToken(tokenizer,TokenType::Comma,optional);
	case '!':
		return matchToken(tokenizer,TokenType::Exclamation,optional);
	case '{':
		return matchToken(tokenizer,TokenType::LBrace,optional);
	case '}':
		return matchToken(tokenizer,TokenType::RBrace,optional);
	case '#':
		return matchToken(tokenizer,TokenType::Hash,optional);
	case '=':
		return matchToken(tokenizer,TokenType::Assign,optional);
	}

	return false;
}

bool ArmParser::parseShift(Tokenizer& tokenizer, ArmOpcodeVariables& vars, bool immediateOnly)
{
	// no shift is also valid
	vars.Shift.UseShift = false;
	if (tokenizer.peekToken().type != TokenType::Comma)
		return true;

	tokenizer.eatToken();

	// load shift mode
	Token& shiftMode = tokenizer.nextToken();
	if (shiftMode.type != TokenType::Identifier)
		return false;

	if (shiftMode.stringValue == L"lsl")
		vars.Shift.Type = 0;
	else if (shiftMode.stringValue == L"lsr")
		vars.Shift.Type = 1;
	else if (shiftMode.stringValue == L"asr")
		vars.Shift.Type = 2;
	else if (shiftMode.stringValue == L"ror")
		vars.Shift.Type = 3;
	else if (shiftMode.stringValue == L"rrx")
		vars.Shift.Type = 4;
	else 
		return false;

	if (parseRegister(tokenizer,vars.Shift.reg) == true)
	{
		if (immediateOnly)
			return false;

		vars.Shift.ShiftByRegister = true;
	} else {
		if (tokenizer.peekToken().type == TokenType::Hash)
			tokenizer.eatToken();
		
		if (parseImmediate(tokenizer,vars.Shift.ShiftExpression) == false)
			return false;

		vars.Shift.ShiftByRegister = false;
	}
	
	vars.Shift.UseShift = true;
	return true;
}

bool ArmParser::parsePseudoShift(Tokenizer& tokenizer, ArmOpcodeVariables& vars, int type)
{
	vars.Shift.Type = type;

	if (parseRegister(tokenizer,vars.Shift.reg) == true)
	{
		vars.Shift.ShiftByRegister = true;
	} else {
		if (tokenizer.peekToken().type == TokenType::Hash)
			tokenizer.eatToken();
		
		if (parseImmediate(tokenizer,vars.Shift.ShiftExpression) == false)
			return false;

		vars.Shift.ShiftByRegister = false;
	}
	
	vars.Shift.UseShift = true;
	return true;
}

int ArmParser::decodeCondition(const std::wstring& text, size_t& pos)
{
	if (pos+2 <= text.size())
	{
		wchar_t c1 = text[pos+0];
		wchar_t c2 = text[pos+1];
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

bool ArmParser::decodeAddressingMode(const std::wstring& text, size_t& pos, unsigned char& dest)
{
	if (pos+2 > text.size())
		return false;

	wchar_t c1 = text[pos+0];	
	wchar_t c2 = text[pos+1];

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

bool ArmParser::decodeXY(const std::wstring& text, size_t& pos, bool& dest)
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

void ArmParser::decodeS(const std::wstring& text, size_t& pos, bool& dest)
{
	dest = pos < text.size() && text[pos] == 's';
	if (dest)
		pos++;
}

bool ArmParser::decodeArmOpcode(const std::wstring& name, const tArmOpcode& opcode, ArmOpcodeVariables& vars)
{
	vars.Opcode.c = vars.Opcode.a = 0;
	vars.Opcode.s = false;

	const u8* encoding = (const u8*) opcode.name;
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

void ArmParser::parseWriteback(Tokenizer& tokenizer, bool& dest)
{
	dest = tokenizer.peekToken().type == TokenType::Exclamation;
	if (dest)
		tokenizer.eatToken();
}

void ArmParser::parsePsr(Tokenizer& tokenizer, bool& dest)
{
	dest = tokenizer.peekToken().type == TokenType::Caret;
	if (dest)
		tokenizer.eatToken();
}

void ArmParser::parseSign(Tokenizer& tokenizer, bool& dest)
{
	switch (tokenizer.peekToken().type)
	{
	case TokenType::Plus:
		dest = true;
		tokenizer.eatToken();
		break;
	case TokenType::Minus:
		dest = false;
		tokenizer.eatToken();
		break;
	default:
		dest = true;
		break;
	}
}

bool ArmParser::parsePsrTransfer(Tokenizer& tokenizer, ArmOpcodeVariables& vars, bool shortVersion)
{
	Token& token = tokenizer.nextToken();
	if (token.type != TokenType::Identifier)
		return false;

	size_t pos = 0;
	if (startsWith(token.stringValue,L"cpsr"))
	{
		vars.PsrData.spsr = false;
		pos = 4;
	} else if (startsWith(token.stringValue,L"spsr"))
	{
		vars.PsrData.spsr = true;
		pos = 4;
	} else {
		return false;
	}

	if (shortVersion)
		return pos == token.stringValue.size();

	if (pos == token.stringValue.size())
	{
		vars.PsrData.field = 0xF;
		return true;
	}

	if (token.stringValue[pos++] != '_')
		return false;

	if (startsWith(token.stringValue,L"ctl",pos))
	{
		vars.PsrData.field = 1;
		return pos+3 == token.stringValue.size();
	} 
	
	if (startsWith(token.stringValue,L"flg",pos))
	{
		vars.PsrData.field = 8;
		return pos+3 == token.stringValue.size();
	}
	
	vars.PsrData.field = 0;
	for (int i = 0; i < 4; i++)
	{
		if (pos == token.stringValue.size())
			break;

		switch(token.stringValue[pos++])
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

bool ArmParser::parseArmParameters(Tokenizer& tokenizer, const tArmOpcode& opcode, ArmOpcodeVariables& vars)
{
	const u8* encoding = (const u8*) opcode.mask;

	ArmRegisterValue tempRegister;

	while (*encoding != 0)
	{
		bool optional = *encoding == '/';
		if (optional)
			encoding++;

		switch (*encoding++)
		{
		case 'd': // register
			CHECK(parseRegister(tokenizer,vars.rd,*encoding++ == '1' ? 14 : 15));
			break;
		case 's': // register
			CHECK(parseRegister(tokenizer,vars.rs,*encoding++ == '1' ? 14 : 15));
			break;
		case 'n': // register
			CHECK(parseRegister(tokenizer,vars.rn,*encoding++ == '1' ? 14 : 15));
			break;
		case 'm': // register
			CHECK(parseRegister(tokenizer,vars.rm,*encoding++ == '1' ? 14 : 15));
			break;
		case 'D': // cop register
			CHECK(parseCopRegister(tokenizer,vars.CopData.cd));
			break;
		case 'N': // cop register
			CHECK(parseCopRegister(tokenizer,vars.CopData.cn));
			break;
		case 'M': // cop register
			CHECK(parseCopRegister(tokenizer,vars.CopData.cm));
			break;
		case 'W':	// writeback
			parseWriteback(tokenizer,vars.writeback);
			break;
		case 'p':	// psr
			parsePsr(tokenizer,vars.psr);
			break;
		case 'P':	// msr/mrs psr data
			CHECK(parsePsrTransfer(tokenizer,vars,*encoding++ == '1'));
			break;
		case 'R':	// register list
			CHECK(parseRegisterList(tokenizer,vars.rlist,0xFFFF));
			encoding += 2;
			break;
		case 'S':	// shift
			CHECK(parseShift(tokenizer,vars,*encoding++ == '1'));
			break;
		case 'I':	// immediate
		case 'i':
			CHECK(parseImmediate(tokenizer,vars.ImmediateExpression));
			vars.ImmediateBitLen = 32;
			break;
		case 'j':	// variable bit immediate
			CHECK(parseImmediate(tokenizer,vars.ImmediateExpression));
			vars.ImmediateBitLen = *encoding++;
			break;
		case 'X': // cop number
			CHECK(parseCopNumber(tokenizer,vars.CopData.cm));
			break;
		case 'Y':	// cop opcode number
			CHECK(parseImmediate(tokenizer,vars.CopData.CpopExpression));
			vars.ImmediateBitLen = 4;
			break;
		case 'Z':	// cop info number
			CHECK(parseImmediate(tokenizer,vars.CopData.CpinfExpression));
			vars.ImmediateBitLen = 3;
			break;
		case 'z':	// shift for pseudo opcodes
			CHECK(parsePseudoShift(tokenizer,vars,*encoding++));
			break;
		case 'v':	// sign for register index parameter
			parseSign(tokenizer,vars.SignPlus);
			break;
		default:
			CHECK(matchSymbol(tokenizer,*(encoding-1),optional));
			break;
		}
	}

	return true;
}

CArmInstruction* ArmParser::parseArmOpcode(Tokenizer& tokenizer)
{
	Token token = tokenizer.nextToken();
	if (token.type != TokenType::Identifier)
		return nullptr;

	ArmOpcodeVariables vars;
	bool paramFail = false;

	for (int z = 0; ArmOpcodes[z].name != NULL; z++)
	{
		if ((ArmOpcodes[z].flags & ARM_ARM9) && !Arm.isArm9())
			continue;

		if (decodeArmOpcode(token.stringValue,ArmOpcodes[z],vars) == true)
		{
			size_t tokenPos = tokenizer.getPosition();

			if (parseArmParameters(tokenizer,ArmOpcodes[z],vars) == true)
			{
				// success, return opcode
				return new CArmInstruction(ArmOpcodes[z],vars);
			}

			tokenizer.setPosition(tokenPos);
			paramFail = true;
		}
	}

	if (paramFail == true)
		Logger::printError(Logger::Error,L"ARM parameter failure");
	else
		Logger::printError(Logger::Error,L"Invalid ARM opcode");

	return nullptr;
}

bool ArmParser::parseThumbParameters(Tokenizer& tokenizer, const tThumbOpcode& opcode, ThumbOpcodeVariables& vars)
{
	const u8* encoding = (const u8*) opcode.mask;

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
			CHECK(parseRegister(tokenizer,vars.rd,7));
			break;
		case 's': // register
			CHECK(parseRegister(tokenizer,vars.rs,7));
			break;
		case 'n': // register
			CHECK(parseRegister(tokenizer,vars.rn,7));
			break;
		case 'o': // register
			CHECK(parseRegister(tokenizer,vars.ro,7));
			break;
		case 'D': // register
			CHECK(parseRegister(tokenizer,vars.rd,15));
			break;
		case 'S': // register
			CHECK(parseRegister(tokenizer,vars.rs,15));
			break;
		case 'r': // forced register
			CHECK(parseRegister(tokenizer,tempRegister,15));
			CHECK(*encoding++ == tempRegister.num);
			break;
		case 'R':	// register list
			value = encoding[0] | (encoding[1] << 8);
			CHECK(parseRegisterList(tokenizer,vars.rlist,value));
			encoding += 2;
			break;
		case 'I':	// immediate
		case 'i':
			CHECK(parseImmediate(tokenizer,vars.ImmediateExpression));
			vars.ImmediateBitLen = *encoding++;
			break;
		default:
			CHECK(matchSymbol(tokenizer,*(encoding-1),optional));
			break;
		}
	}

	return true;
}

CThumbInstruction* ArmParser::parseThumbOpcode(Tokenizer& tokenizer)
{
	Token token = tokenizer.nextToken();
	if (token.type != TokenType::Identifier)
		return nullptr;

	ThumbOpcodeVariables vars;
	bool paramFail = false;

	for (int z = 0; ThumbOpcodes[z].name != NULL; z++)
	{
		if ((ThumbOpcodes[z].flags & THUMB_ARM9) && !Arm.isArm9())
			continue;

		// todo: save as wchar
		std::wstring name = convertUtf8ToWString(ThumbOpcodes[z].name);

		if (token.stringValue == name)
		{
			size_t tokenPos = tokenizer.getPosition();
			
			if (parseThumbParameters(tokenizer,ThumbOpcodes[z],vars) == true)
			{
				// success, return opcode
				return new CThumbInstruction(ThumbOpcodes[z],vars);
			}

			tokenizer.setPosition(tokenPos);
			paramFail = true;
		}
	}

	if (paramFail == true)
		Logger::printError(Logger::Error,L"THUMB parameter failure");
	else
		Logger::printError(Logger::Error,L"Invalid THUMB opcode");
	
	return false;
}
