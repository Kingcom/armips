#include "stdafx.h"
#include "MipsParser.h"
#include "Parser/Parser.h"
#include "Parser/ExpressionParser.h"
#include "Util/Util.h"
#include "Core/Common.h"

#define CHECK(exp) if (!(exp)) return false;

const MipsRegisterDescriptor mipsRegisters[] = {
	{ L"r0", 0 },	{ L"zero", 0},	{ L"at", 1 },	{ L"r1", 1 },
	{ L"v0", 2 },	{ L"r2", 2 },	{ L"v1", 3 },	{ L"r3", 3 },
	{ L"a0", 4 },	{ L"r4", 4 },	{ L"a1", 5 },	{ L"r5", 5 },
	{ L"a2", 6 },	{ L"r6", 6 },	{ L"a3", 7 },	{ L"r7", 7 },
	{ L"t0", 8 },	{ L"r8", 8 },	{ L"t1", 9 },	{ L"r9", 9 },
	{ L"t2", 10 },	{ L"r10", 10 },	{ L"t3", 11 },	{ L"r11", 11 },
	{ L"t4", 12 },	{ L"r12", 12 },	{ L"t5", 13 },	{ L"r13", 13 },
	{ L"t6", 14 },	{ L"r14", 14 },	{ L"t7", 15 },	{ L"r15", 15 },
	{ L"s0", 16 },	{ L"r16", 16 },	{ L"s1", 17 },	{ L"r17", 17 },
	{ L"s2", 18 },	{ L"r18", 18 },	{ L"s3", 19 },	{ L"r19", 19 },
	{ L"s4", 20 },	{ L"r20", 20 },	{ L"s5", 21 },	{ L"r21", 21 },
	{ L"s6", 22 },	{ L"r22", 22 },	{ L"s7", 23 },	{ L"r23", 23 },
	{ L"t8", 24 },	{ L"r24", 24 },	{ L"t9", 25 },	{ L"r25", 25 },
	{ L"k0", 26 },	{ L"r26", 26 },	{ L"k1", 27 },	{ L"r27", 27 },
	{ L"gp", 28 },	{ L"r28", 28 },	{ L"sp", 29 },	{ L"r29", 29 },
	{ L"fp", 30 },	{ L"r30", 30 },	{ L"ra", 31 },	{ L"r31", 31 },
};

const MipsRegisterDescriptor mipsFloatRegisters[] = {
	{ L"f0", 0 },	{ L"f1", 1 },	{ L"f2", 2 },	{ L"f3", 3 },
	{ L"f4", 4 },	{ L"f5", 5 },	{ L"f6", 6 },	{ L"f7", 7 },
	{ L"f8", 8 },	{ L"f9", 9 },	{ L"f00", 0 },	{ L"f01", 1 },
	{ L"f02", 2 },	{ L"f03", 3 },	{ L"f04", 4 },	{ L"f05", 5 },
	{ L"f06", 6 },	{ L"f07", 7 },	{ L"f08", 8 },	{ L"f09", 9 },
	{ L"f10", 10 },	{ L"f11", 11 },	{ L"f12", 12 },	{ L"f13", 13 },
	{ L"f14", 14 },	{ L"f15", 15 },	{ L"f16", 16 },	{ L"f17", 17 },
	{ L"f18", 18 },	{ L"f19", 19 },	{ L"f20", 20 },	{ L"f21", 21 },
	{ L"f22", 22 },	{ L"f23", 23 },	{ L"f24", 24 },	{ L"f25", 25 },
	{ L"f26", 26 },	{ L"f27", 27 },	{ L"f28", 28 },	{ L"f29", 29 },
	{ L"f30", 30 },	{ L"f31", 31 },
};

const MipsRegisterDescriptor mipsPs2Cop2FpRegisters[] = {
	{ L"vf0", 0 },		{ L"vf1", 1 },		{ L"vf2", 2 },		{ L"vf3", 3 },
	{ L"vf4", 4 },		{ L"vf5", 5 },		{ L"vf6", 6 },		{ L"vf7", 7 },
	{ L"vf8", 8 },		{ L"vf9", 9 },		{ L"vf00", 0 },		{ L"vf01", 1 },
	{ L"vf02", 2 },		{ L"vf03", 3 },		{ L"vf04", 4 },		{ L"vf05", 5 },
	{ L"vf06", 6 },		{ L"vf07", 7 },		{ L"vf08", 8 },		{ L"vf09", 9 },
	{ L"vf10", 10 },	{ L"vf11", 11 },	{ L"vf12", 12 },	{ L"vf13", 13 },
	{ L"vf14", 14 },	{ L"vf15", 15 },	{ L"vf16", 16 },	{ L"vf17", 17 },
	{ L"vf18", 18 },	{ L"vf19", 19 },	{ L"vf20", 20 },	{ L"vf21", 21 },
	{ L"vf22", 22 },	{ L"vf23", 23 },	{ L"vf24", 24 },	{ L"vf25", 25 },
	{ L"vf26", 26 },	{ L"vf27", 27 },	{ L"vf28", 28 },	{ L"vf29", 29 },
	{ L"vf30", 30 },	{ L"vf31", 31 },
};

CAssemblerCommand* MipsParser::parseDirective(Tokenizer& tokenizer)
{
	return nullptr;
}

bool MipsParser::parseRegisterTable(Tokenizer& tokenizer, MipsRegisterValue& dest, const MipsRegisterDescriptor* table, size_t count)
{
	Token& token = tokenizer.peekToken();
	bool hasDollar = token.type == TokenType::Dollar;
	if (hasDollar)
		token = tokenizer.peekToken(1);

	if (token.type != TokenType::Identifier)
		return false;

	for (size_t i = 0; i < count; i++)
	{
		if (token.stringValue == table[i].name)
		{
			dest.name = token.stringValue;
			dest.num = table[i].num;
			tokenizer.eatTokens(hasDollar ? 2 : 1);
			return true;
		}
	}

	return false;
}

bool MipsParser::parseRegister(Tokenizer& tokenizer, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::Normal;
	return parseRegisterTable(tokenizer,dest,mipsRegisters,ARRAY_SIZE(mipsRegisters));
}

bool MipsParser::parseFpuRegister(Tokenizer& tokenizer, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::Float;
	return parseRegisterTable(tokenizer,dest,mipsFloatRegisters,ARRAY_SIZE(mipsFloatRegisters));
}

bool MipsParser::parsePs2Cop2Register(Tokenizer& tokenizer, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::Ps2Cop2;
	return parseRegisterTable(tokenizer,dest,mipsPs2Cop2FpRegisters,ARRAY_SIZE(mipsPs2Cop2FpRegisters));
}

static bool decodeDigit(wchar_t digit, int& dest)
{
	if (digit >= '0' && digit <= '9')
	{
		dest = digit-'0';
		return true;
	}
	return false;
}

bool MipsParser::parseVfpuRegister(Tokenizer& tokenizer, MipsRegisterValue& reg, int size)
{
	Token& token = tokenizer.peekToken();
	if (token.type != TokenType::Identifier || token.stringValue.size() != 4)
		return false;

	int mtx,col,row;
	if (decodeDigit(token.stringValue[1],mtx) == false) return false;
	if (decodeDigit(token.stringValue[2],col) == false) return false;
	if (decodeDigit(token.stringValue[3],row) == false) return false;
	wchar_t mode = towlower(token.stringValue[0]);

	if (size < 0 || size > 3)
		return false;

	if (row > 3 || col > 3 || mtx > 7)
		return false;

	reg.num = 0;
	switch (mode)
	{
	case 'r':					// transposed vector
		reg.num |= (1 << 5);
		std::swap(col,row);		// fallthrough
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
		reg.num |= (1 << 5);	// fallthrough
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
			if ( row & ~1)
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

	reg.name = token.stringValue;
	return true;
}

bool MipsParser::parseVfpuControlRegister(Tokenizer& tokenizer, MipsRegisterValue& reg)
{
	static const wchar_t* vfpuCtrlNames[16] = {
		L"spfx",	L"tpfx",	L"dpfx",	L"cc",
		L"inf4",	L"rsv5",	L"rsv6",	L"rev",
		L"rcx0",	L"rcx1",	L"rcx2",	L"rcx3",
		L"rcx4",	L"rcx5",	L"rcx6",	L"rcx7",
	};

	Token& token = tokenizer.peekToken();

	if (token.type == TokenType::Identifier)
	{
		for (int i = 0; i < 16; i++)
		{
			if (token.stringValue == vfpuCtrlNames[i])
			{
				reg.num = i;
				reg.name = vfpuCtrlNames[i];

				tokenizer.eatToken();
				return true;
			}
		}
	} else if (token.type == TokenType::Integer && token.intValue <= 15)
	{
		reg.num = (int) token.intValue;
		reg.name = vfpuCtrlNames[reg.num];

		tokenizer.eatToken();
		return true;
	}

	return false;
}

bool MipsParser::parseImmediate(Tokenizer& tokenizer, Expression& dest)
{
	// check for (reg) sequence
	if (tokenizer.peekToken().type == TokenType::LParen)
	{
		MipsRegisterValue tempValue;
		
		size_t pos = tokenizer.getPosition();
		tokenizer.eatToken();

		bool isRegister = parseRegister(tokenizer,tempValue);
		tokenizer.setPosition(pos);

		if (isRegister)
			return false;
	}

	dest = parseExpression(tokenizer);
	return dest.isLoaded();
}

bool MipsParser::matchSymbol(Tokenizer& tokenizer, wchar_t symbol)
{
	switch (symbol)
	{
	case '(':
		return matchToken(tokenizer,TokenType::LParen);
	case ')':
		return matchToken(tokenizer,TokenType::RParen);
	case ',':
		return matchToken(tokenizer,TokenType::Comma);
	}

	return false;
}

bool MipsParser::parseVcstParameter(Tokenizer& tokenizer, int& result)
{
	static TokenSequenceParser sequenceParser;

	// initialize on first use
	if (sequenceParser.getEntryCount() == 0)
	{
		// maxfloat
		sequenceParser.addEntry(1,
			{TokenType::Identifier},
			{L"maxfloat"}
		);
		// sqrt(2)
		sequenceParser.addEntry(2,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen},
			{L"sqrt", 2ull}
		);
		// sqrt(1/2)
		sequenceParser.addEntry(3,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::Div, TokenType::Integer, TokenType::RParen},
			{L"sqrt", 1ull, 2ull}
		);
		// sqrt(0.5)
		sequenceParser.addEntry(3,
			{TokenType::Identifier, TokenType::LParen, TokenType::Float, TokenType::RParen},
			{L"sqrt", 0.5}
		);
		// 2/sqrt(pi)
		sequenceParser.addEntry(4,
			{TokenType::Integer, TokenType::Div, TokenType::Identifier, TokenType::LParen, TokenType::Identifier, TokenType::RParen},
			{2ull, L"sqrt", L"pi"}
		);
		// 2/pi
		sequenceParser.addEntry(5,
			{TokenType::Integer, TokenType::Div, TokenType::Identifier},
			{2ull, L"pi"}
		);
		// 1/pi
		sequenceParser.addEntry(6,
			{TokenType::Integer, TokenType::Div, TokenType::Identifier},
			{1ull, L"pi"}
		);
		// pi/4
		sequenceParser.addEntry(7,
			{TokenType::Identifier, TokenType::Div, TokenType::Integer},
			{L"pi", 4ull}
		);
		// pi/2
		sequenceParser.addEntry(8,
			{TokenType::Identifier, TokenType::Div, TokenType::Integer},
			{L"pi", 2ull}
		);
		// pi/6 - early because "pi" is a prefix of it
		sequenceParser.addEntry(16,
			{TokenType::Identifier, TokenType::Div, TokenType::Integer},
			{L"pi", 6ull}
		);
		// pi
		sequenceParser.addEntry(9,
			{TokenType::Identifier},
			{L"pi"}
		);
		// e
		sequenceParser.addEntry(10,
			{TokenType::Identifier},
			{L"e"}
		);
		// log2(e)
		sequenceParser.addEntry(11,
			{TokenType::Identifier, TokenType::LParen, TokenType::Identifier, TokenType::RParen},
			{L"log2", L"e"}
		);
		// log10(e)
		sequenceParser.addEntry(12,
			{TokenType::Identifier, TokenType::LParen, TokenType::Identifier, TokenType::RParen},
			{L"log10", L"e"}
		);
		// ln(2)
		sequenceParser.addEntry(13,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen},
			{L"ln", 2ull}
		);
		// ln(10)
		sequenceParser.addEntry(14,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen},
			{L"ln", 10ull}
		);
		// 2*pi
		sequenceParser.addEntry(15,
			{TokenType::Integer, TokenType::Mult, TokenType::Identifier},
			{2ull, L"pi"}
		);
		// log10(2)
		sequenceParser.addEntry(17,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen},
			{L"log10", 2ull}
		);
		// log2(10)
		sequenceParser.addEntry(18,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen},
			{L"log2", 10ull}
		);
		// sqrt(3)/2
		sequenceParser.addEntry(19,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen, TokenType::Div, TokenType::Integer},
			{L"sqrt", 3ull, 2ull}
		);
	}

	return sequenceParser.parse(tokenizer,result);
}

bool MipsParser::parseVfpuVrot(Tokenizer& tokenizer, int& result, int size)
{
	int sin = -1;
	int cos = -1;
	bool negSine = false;
	int sineCount = 0;

	if (tokenizer.nextToken().type != TokenType::LBrack)
		return false;
	
	int numElems = size+1;
	for (int i = 0; i < numElems; i++)
	{
		Token& token = tokenizer.nextToken();
		
		if (i != 0)
		{
			if (token.type != TokenType::Comma)
				return false;

			token = tokenizer.nextToken();
		}

		bool isNeg = token.type == TokenType::Minus;
		if (isNeg)
			token = tokenizer.nextToken();

		if (token.type != TokenType::Identifier || token.stringValue.size() != 1)
			return false;

		switch (token.stringValue[0])
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
	
	if (tokenizer.nextToken().type != TokenType::RBrack)
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

bool MipsParser::parseVfpuCondition(Tokenizer& tokenizer, int& result)
{
	static const wchar_t* conditions[] = {
		L"fl", L"eq", L"lt", L"le", L"tr", L"ne", L"ge", L"gt",
		L"ez", L"en", L"ei", L"es", L"nz", L"nn", L"ni", L"ns"
	};

	Token& token = tokenizer.nextToken();
	if (token.type != TokenType::Identifier)
		return false;

	for (int i = 0; i < ARRAY_SIZE(conditions); i++)
	{
		if (token.stringValue == conditions[i])
		{
			result = i;
			return true;
		}
	}

	return false;
}

bool MipsParser::parseVpfxsParameter(Tokenizer& tokenizer, int& result)
{
	static TokenSequenceParser sequenceParser;

	// initialize on first use
	if (sequenceParser.getEntryCount() == 0)
	{
		// 0
		sequenceParser.addEntry(0, {TokenType::Integer}, {0ull} );
		// 1
		sequenceParser.addEntry(1, {TokenType::Integer}, {1ull} );
		// 2
		sequenceParser.addEntry(2, {TokenType::Integer}, {2ull} );
		// 1/2
		sequenceParser.addEntry(3, {TokenType::Integer, TokenType::Div, TokenType::Integer}, {1ull, 2ull} );
		// 3
		sequenceParser.addEntry(4, {TokenType::Integer}, {3ull} );
		// 1/3
		sequenceParser.addEntry(5, {TokenType::Integer, TokenType::Div, TokenType::Integer}, {1ull, 3ull} );
		// 1/4
		sequenceParser.addEntry(6, {TokenType::Integer, TokenType::Div, TokenType::Integer}, {1ull, 4ull} );
		// 1/6
		sequenceParser.addEntry(7, {TokenType::Integer, TokenType::Div, TokenType::Integer}, {1ull, 6ull} );
	}

	if (tokenizer.nextToken().type != TokenType::LBrack)
		return false;
	
	for (int i = 0; i < 4; i++)
	{
		Token& token = tokenizer.nextToken();

		if (i != 0)
		{
			if (token.type != TokenType::Comma)
				return false;

			token = tokenizer.nextToken();
		}
		
		// negation
		if (token.type == TokenType::Minus)
		{
			result |= 1 << (16+i);
			token = tokenizer.nextToken();
		}

		// abs
		bool abs = false;
		if (token.type == TokenType::BitOr)
		{
			result |= 1 << (8+i);
			abs = true;
			token = tokenizer.nextToken();
		}
		
		// check for register
		const wchar_t* reg;
		static const wchar_t* vpfxstRegisters = L"xyzw";
		if (token.stringValue.size() == 1 && (reg = wcschr(vpfxstRegisters,token.stringValue[0])) != nullptr)
		{
			result |= (reg-vpfxstRegisters) << (i*2);

			if (abs && tokenizer.nextToken().type != TokenType::BitOr)
				return false;

			continue;
		}
		
		// abs is invalid with constants
		if (abs)
			return false;

		result |= 1 << (12+i);

		int constNum = -1;
		if (sequenceParser.parse(tokenizer,constNum) == false)
			return false;
		
		result |= (constNum & 3) << (i*2);
		if (constNum & 4)
			result |= 1 << (8+i);
	}

	return tokenizer.nextToken().type == TokenType::RBrack;
}

bool MipsParser::parseVpfxdParameter(Tokenizer& tokenizer, int& result)
{
	static TokenSequenceParser sequenceParser;

	// initialize on first use
	if (sequenceParser.getEntryCount() == 0)
	{
		// 0-1
		sequenceParser.addEntry(1,
			{TokenType::Integer, TokenType::Minus, TokenType::Integer},
			{0ull, 1ull} );
		// 0-1
		sequenceParser.addEntry(-1,
			{TokenType::Integer, TokenType::Minus, TokenType::NumberString},
			{0ull, L"1m"} );
		// 0:1
		sequenceParser.addEntry(1,
			{TokenType::Integer, TokenType::Colon, TokenType::Integer},
			{0ull, 1ull} );
		// 0:1
		sequenceParser.addEntry(-1,
			{TokenType::Integer, TokenType::Colon, TokenType::NumberString},
			{0ull, L"1m"} );
		// -1-1
		sequenceParser.addEntry(3,
			{TokenType::Minus, TokenType::Integer, TokenType::Minus, TokenType::Integer},
			{1ull, 1ull} );
		// -1-1m
		sequenceParser.addEntry(-3,
			{TokenType::Minus, TokenType::Integer, TokenType::Minus, TokenType::NumberString},
			{1ull, L"1m"} );
		// -1:1
		sequenceParser.addEntry(3,
			{TokenType::Minus, TokenType::Integer, TokenType::Colon, TokenType::Integer},
			{1ull, 1ull} );
		// -1:1m
		sequenceParser.addEntry(-3,
			{TokenType::Minus, TokenType::Integer, TokenType::Colon, TokenType::NumberString},
			{1ull, L"1m"} );
	}

	for (int i = 0; i < 4; i++)
	{
		Token& token = tokenizer.nextToken();

		if (i != 0)
		{
			if (token.type != TokenType::Comma)
				return false;

			token = tokenizer.nextToken();
		}
		
		int num = 0;
		if (sequenceParser.parse(tokenizer,num) == false)
			return false;

		// m versions
		if (num < 0)
		{
			result |= 1 << (8+i);
			num = abs(num);
		}

		result |= num << (2*i);
	}
	
	return tokenizer.nextToken().type == TokenType::RBrack;
}


bool MipsParser::decodeCop2BranchCondition(const std::wstring& text, size_t& pos, int& result)
{
	if (pos+3 == text.size())
	{
		if (startsWith(text,L"any",pos))
		{
			result = 4;
			pos += 3;
			return true;
		}
		if (startsWith(text,L"all",pos))
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

bool MipsParser::parseCop2BranchCondition(Tokenizer& tokenizer, int& result)
{
	Token& token = tokenizer.nextToken();

	if (token.type == TokenType::Integer)
	{
		result = (int) token.intValue;
		return token.intValue <= 5;
	}

	if (token.type != TokenType::Identifier)
		return false;

	size_t pos = 0;
	return decodeCop2BranchCondition(token.stringValue,pos,result);
}

bool MipsParser::parseWb(Tokenizer& tokenizer)
{
	Token& token = tokenizer.nextToken();
	if (token.type != TokenType::Identifier)
		return false;

	return token.stringValue == L"wb";
}

bool MipsParser::decodeImmediateSize(const u8*& encoding, MipsImmediateType& dest)
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
		case 8:
			dest = MipsImmediateType::Immediate8;
			break;
		case 16:
			dest = MipsImmediateType::Immediate16;
			break;
		case 20:
			dest = MipsImmediateType::Immediate20;
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

bool MipsParser::decodeVfpuType(const std::wstring& name, size_t& pos, int& dest)
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

bool MipsParser::decodeOpcode(const std::wstring& name, const tMipsOpcode& opcode)
{
	const u8* encoding = (const u8*) opcode.name;
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
}

bool MipsParser::parseParameters(Tokenizer& tokenizer, const tMipsOpcode& opcode)
{
	const u8* encoding = (const u8*) opcode.encoding;

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
			CHECK(parseRegister(tokenizer,registers.grt));
			break;
		case 'd':	// register
			CHECK(parseRegister(tokenizer,registers.grd));
			break;
		case 's':	// register
			CHECK(parseRegister(tokenizer,registers.grs));
			break;
		case 'T':	// float register
			CHECK(parseRegister(tokenizer,registers.frt));
			break;
		case 'D':	// float register
			CHECK(parseRegister(tokenizer,registers.frd));
			break;
		case 'S':	// float register
			CHECK(parseRegister(tokenizer,registers.frs));
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
				CHECK(parseVfpuRegister(tokenizer,registers.vrs,actualSize));
				CHECK(registers.vrs.type == MipsRegisterType::VfpuVector);
				if (opcode.flags & MO_VFPU_6BIT) CHECK(!(registers.vrs.num & 0x40));
				break;
			case 't':
				CHECK(parseVfpuRegister(tokenizer,registers.vrt,actualSize));
				CHECK(registers.vrt.type == MipsRegisterType::VfpuVector);
				if (opcode.flags & MO_VFPU_6BIT) CHECK(!(registers.vrt.num & 0x40));
				break;
			case 'd':
				CHECK(parseVfpuRegister(tokenizer,registers.vrd,actualSize));
				CHECK(registers.vrd.type == MipsRegisterType::VfpuVector);
				if (opcode.flags & MO_VFPU_6BIT) CHECK(!(registers.vrd.num & 0x40));
				break;
			case 'c':
				CHECK(parseVfpuControlRegister(tokenizer,registers.vrd));
				break;
			default:
				return false;
			}
			break;
		case 'm':	// vfpu amtrix register
			switch (*encoding++)
			{
			case 's':
				CHECK(parseVfpuRegister(tokenizer,registers.vrs,opcodeData.vfpuSize));
				CHECK(registers.vrs.type == MipsRegisterType::VfpuMatrix);
				if (opcode.flags & MO_TRANSPOSE_VS)
					registers.vrs.num ^= 0x20;
				break;
			case 't':
				CHECK(parseVfpuRegister(tokenizer,registers.vrt,opcodeData.vfpuSize));
				CHECK(registers.vrt.type == MipsRegisterType::VfpuMatrix);
				break;
			case 'd':
				CHECK(parseVfpuRegister(tokenizer,registers.vrd,opcodeData.vfpuSize));
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
				CHECK(parseRegister(tokenizer,registers.ps2vrt));
				break;
			case 'd':	// register
				CHECK(parseRegister(tokenizer,registers.ps2vrd));
				break;
			case 's':	// register
				CHECK(parseRegister(tokenizer,registers.ps2vrs));
				break;
			default:
				return false;
			}
			break;
		case 'r':	// forced register
			CHECK(parseRegister(tokenizer,tempRegister));
			CHECK(tempRegister.num == *encoding++);
			break;
		case 'i':	// primary immediate
			CHECK(parseImmediate(tokenizer,immediate.primary.expression));
			CHECK(decodeImmediateSize(encoding,immediate.primary.type));
			break;
		case 'j':	// secondary immediate
			switch (*encoding++)
			{
			case 'e':
				CHECK(parseImmediate(tokenizer,immediate.secondary.expression));
				immediate.secondary.type = MipsImmediateType::Ext;
				break;
			case 'i':
				CHECK(parseImmediate(tokenizer,immediate.secondary.expression));
				immediate.secondary.type = MipsImmediateType::Ins;
				break;
			case 'b':
				CHECK(parseCop2BranchCondition(tokenizer,immediate.secondary.originalValue));
				immediate.secondary.type = MipsImmediateType::Cop2BranchType;
				immediate.secondary.value = immediate.secondary.originalValue;
				break;
			default:
				return false;
			}
			break;
		case 'C':	// vfpu condition
			CHECK(parseVfpuCondition(tokenizer,opcodeData.vectorCondition));
			break;
		case 'W':	// vfpu argument
			switch (*encoding++)
			{
			case 's':
				CHECK(parseVpfxsParameter(tokenizer,immediate.primary.originalValue));
				immediate.primary.value = immediate.primary.originalValue;
				immediate.primary.type = MipsImmediateType::Immediate20_0;
				break;
			case 'd':
				CHECK(parseVpfxdParameter(tokenizer,immediate.primary.originalValue));
				immediate.primary.value = immediate.primary.originalValue;
				immediate.primary.type = MipsImmediateType::Immediate16;
				break;
			case 'c':
				CHECK(parseVcstParameter(tokenizer,immediate.primary.originalValue));
				immediate.primary.value = immediate.primary.originalValue;
				immediate.primary.type = MipsImmediateType::Immediate5;
				break;
			case 'r':
				CHECK(parseVfpuVrot(tokenizer,immediate.primary.originalValue,opcodeData.vfpuSize));
				immediate.primary.value = immediate.primary.originalValue;
				immediate.primary.type = MipsImmediateType::Immediate5;
				break;
			default:
				return false;
			}
			break;
		case 'w':	// 'wb' characters
			CHECK(parseWb(tokenizer));
			break;
		default:
			CHECK(matchSymbol(tokenizer,*(encoding-1)));
			break;
		}
	}

	opcodeData.opcode = opcode;
	setOmittedRegisters(opcode);
	return true;
}

CMipsInstruction* MipsParser::parseOpcode(Tokenizer& tokenizer)
{
	Token token = tokenizer.nextToken();
	if (token.type != TokenType::Identifier)
		return nullptr;

	bool paramFail = false;
	const MipsArchDefinition& arch = mipsArchs[Mips.GetVersion()];

	for (int z = 0; MipsOpcodes[z].name != NULL; z++)
	{
		if ((MipsOpcodes[z].archs & arch.supportSets) == 0)
			continue;
		if ((MipsOpcodes[z].archs & arch.excludeMask) != 0)
			continue;

		if ((MipsOpcodes[z].flags & MO_64BIT) && !(arch.flags & MO_64BIT))
			continue;
		if ((MipsOpcodes[z].flags & MO_FPU) && !(arch.flags & MO_FPU))
			continue;

		if (decodeOpcode(token.stringValue,MipsOpcodes[z]) == true)
		{
			size_t tokenPos = tokenizer.getPosition();

			if (parseParameters(tokenizer,MipsOpcodes[z]) == true)
			{
				// success, return opcode
				return new CMipsInstruction(opcodeData,immediate,registers);
			}

			tokenizer.setPosition(tokenPos);
			paramFail = true;
		}
	}

	if (paramFail == true)
		Logger::printError(Logger::Error,L"MIPS parameter failure");
	else
		Logger::printError(Logger::Error,L"Invalid MIPS opcode");

	return nullptr;
}
