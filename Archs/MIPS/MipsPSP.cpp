#include "stdafx.h"
#include "MipsPSP.h"

static bool decodeDigit(char digit, int& dest)
{
	if (digit >= '0' && digit <= '9')
	{
		dest = digit-'0';
		return true;
	}
	return false;
}

bool parseVFPURegister(const char* line, MipsVFPURegister& reg, int size)
{
	int mtx,col,row;
	if (decodeDigit(line[1],mtx) == false) return false;
	if (decodeDigit(line[2],col) == false) return false;
	if (decodeDigit(line[3],row) == false) return false;
	char mode = tolower(line[0]);

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
		reg.type = MIPSVFPU_VECTOR;

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
		reg.type = MIPSVFPU_VECTOR;

		if (size != 0)
			return false;
		break;
	case 'e':					// transposed matrix
		reg.num |= (1 << 5);	// fallthrough
	case 'm':					// matrix
		reg.type = MIPSVFPU_MATRIX;

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
	memcpy(reg.name,line,4);
	reg.name[4] = 0;
	return true;
}

int parseVFPUCondition(const char* source, int& RetLen)
{
	if (source[0] == 0 || source[1] == 0)
		return -1;

	if (source[2] == ',' || source[2] == '\n' || source[2] == 0
		|| source[2] == ')' || source[2] == '(' || source[2] == '-')
	{
		static const char *conditions[] = {"fl", "eq", "lt", "le", "tr", "ne", "ge", "gt", "ez", "en", "ei",
			"es", "nz", "nn", "ni", "ns"};
		for (int i = 0; i < (int)sizeof(conditions)/sizeof(conditions[0]); ++i)
		{
			if (source[0] == conditions[i][0] && source[1] == conditions[i][1])
			{
				RetLen = 2;
				return i;
			}
		}
	}

	return -1;
}

static const char* vpfxstRegisters = "xyzw";
static const char* vpfxstConstants[8] = {"0","1","2","1/2","3","1/3","1/4","1/6"};

bool parseVpfxsParameter(const char* text, int& result, int& RetLen)
{
	const char* start = text;
	result = 0;

	if (*text++ != '[')
		return false;

	for (int i = 0; i < 4; i++)
	{
		char buffer[64];
		
		if (*text == 0 || *text == ']')
			return false;

		// extract element from text, so we don't have to worry about whitespace
		int pos = 0;
		while (*text != ',' && *text != 0 &&  *text != ']')
		{
			if (*text == ' ' || *text == '\t')
			{
				text++;
				continue;
			}

			buffer[pos++] = *text++;
		}

		if (*text == ',')
			text++;

		if (pos == 0)
			return false;

		buffer[pos] = 0;
		pos = 0;
		
		// negation
		if (buffer[pos] == '-')
		{
			result |= 1 << (16+i);
			pos++;
		}

		// abs
		bool abs = false;
		if (buffer[pos] == '|')
		{
			result |= 1 << (8+i);
			abs = true;
			pos++;
		}

		// check for register
		const char* reg;
		if ((reg = strchr(vpfxstRegisters,buffer[pos])) != NULL)
		{
			result |= (reg-vpfxstRegisters) << (i*2);

			if (abs && buffer[pos+1] != '|')
				return false;

			continue;
		}

		// abs is invalid with constants
		if (abs)
			return false;

		result |= 1 << (12+i);

		int constNum = -1;
		for (int k = 0; k < 8; k++)
		{
			if (strcmp(&buffer[pos],vpfxstConstants[k]) == 0)
			{
				constNum = k;
				break;
			}
		}

		if (constNum == -1)
			return false;

		result |= (constNum & 3) << (i*2);
		if (constNum & 4)
			result |= 1 << (8+i);
	}
	
	if (*text++ != ']')
		return false;

	RetLen = (int) (text-start);
	return true;
}

bool parseVpfxdParameter(const char* text, int& result, int& RetLen)
{
	const char* start = text;
	result = 0;

	if (*text++ != '[')
		return false;

	for (int i = 0; i < 4; i++)
	{
		char buffer[64];
		
		if (*text == 0 || *text == ']')
		{
			if (i == 3)
				break;
			return false;
		}

		// extract element from text, so we don't have to worry about whitespace
		int pos = 0;
		while (*text != ',' && *text != 0 &&  *text != ']')
		{
			if (*text == ' ' || *text == '\t')
			{
				text++;
				continue;
			}

			buffer[pos++] = *text++;
		}

		if (*text == ',')
			text++;

		if (pos == 0)
			continue;

		buffer[pos] = 0;
		int length = pos;
		pos = 0;
		
		if (length > 0 && buffer[length-1] == 'm')
		{
			buffer[--length] = 0;
			result |= 1 << (8+i);
		}

		if (strcmp(buffer,"0-1") == 0 || strcmp(buffer,"0:1") == 0)
			result |= 1 << (2*i);
		else if (strcmp(buffer,"-1-1") == 0 || strcmp(buffer,"-1:1") == 0)
			result |= 3 << (2*i);
	}
	
	if (*text++ != ']')
		return false;

	RetLen = (int) (text-start);
	return true;
}

bool parseVcstParameter(const char* text, int& result, int& RetLen)
{
	static const char *constants[32] = 
	{
		"(undef)",
		"maxfloat",
		"sqrt(2)",
		"sqrt(1/2)",
		"2/sqrt(pi)",
		"2/pi",
		"1/pi",
		"pi/4",
		"pi/2",
		"pi",
		"e",
		"log2(e)",
		"log10(e)",
		"ln(2)",
		"ln(10)",
		"2*pi",
		"pi/6",
		"log10(2)",
		"log2(10)",
		"sqrt(3)/2"
	};

	for (int i = 1; i < 32; i++)
	{
		if (strcmp(text,constants[i]) == 0)
		{
			result = i;
			RetLen = (int) strlen(text);
			return true;
		}
	}

	return false;
}

bool parseCop2BranchCondition(const char* text, int& result, int& RetLen)
{
	switch (*text)
	{
	case '0':
	case 'x':
		result = 0;
		RetLen = 1;
		return true;
	case '1':
	case 'y':
		result = 1;
		RetLen = 1;
		return true;
	case '2':
	case 'z':
		result = 2;
		RetLen = 1;
		return true;
	case '3':
	case 'w':
		result = 3;
		RetLen = 1;
		return true;
	case '4':
	case '5':
		result = *text - '0';
		RetLen = 1;
		return true;
	}

	if (memcmp(text,"any",3) == 0)
	{
		result = 4;
		RetLen = 3;
		return true;
	}
	
	if (memcmp(text,"all",3) == 0)
	{
		result = 5;
		RetLen = 3;
		return true;
	}

	return false;
}

bool parseVfpuControlRegister(const char* text, MipsVFPURegister& reg, int& RetLen)
{
	static const char* vfpuCtrlNames[16] = {
		"spfx",
		"tpfx",
		"dpfx",
		"cc",
		"inf4",
		"rsv5",
		"rsv6",
		"rev",
		"rcx0",
		"rcx1",
		"rcx2",
		"rcx3",
		"rcx4",
		"rcx5",
		"rcx6",
		"rcx7",
	};

	for (int i = 0; i < 16; i++)
	{
		if (strcmp(text,vfpuCtrlNames[i]) == 0)
		{
			reg.num = i;
			strcpy(reg.name,vfpuCtrlNames[i]);
			RetLen = (int) strlen(vfpuCtrlNames[i]);
			return true;
		}
	}

	RetLen = 0;
	reg.num = 0;
	while (*text != 0 && *text != ',')
	{
		if (*text < '0' || *text > '9')
			return false;
		reg.num = (reg.num*10) + *text - '0';
		
		RetLen++;
		text++;
	}

	if (reg.num > 15)
		return false;
	
	strcpy(reg.name,vfpuCtrlNames[reg.num ]);
	return true;
}

int parseVfpuVrot(const char* text, int& result, int size, int& RetLen)
{
	const char* start = text;

	int sin = -1;
	int cos = -1;
	bool negSine = false;
	int sineCount = 0;

	if (*text++ != '[')
		return false;
	
	int numElems = size+1;
	for (int i = 0; i < numElems; i++)
	{
		char buffer[64];
		
		if (*text == 0 || *text == ']')
			return false;

		// extract element from text, so we don't have to worry about whitespace
		int pos = 0;
		while (*text != ',' && *text != 0 &&  *text != ']')
		{
			if (*text == ' ' || *text == '\t')
			{
				text++;
				continue;
			}

			buffer[pos++] = *text++;
		}

		if (*text == ',')
			text++;

		if (pos == 0)
			return false;

		buffer[pos] = 0;
		pos = 0;
		
		bool isNeg = buffer[pos] == '-';
		if (isNeg)
			pos++;

		switch (buffer[pos++])
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

		if (buffer[pos] != 0)
			return false;
	}
	
	if (*text++ != ']')
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

	RetLen = (int) (text-start);
	return true;
}

// http://code.google.com/p/jpcsp/source/browse/trunk/src/jpcsp/Allegrex/VfpuState.java?spec=svn3676&r=3383#1196
int floatToHalfFloat(int i)
{
	int s = ((i >> 16) & 0x00008000); // sign
	int e = ((i >> 23) & 0x000000ff) - (127 - 15); // exponent
	int f = ((i >> 0) & 0x007fffff); // fraction

	// need to handle NaNs and Inf?
	if (e <= 0) {
		if (e < -10) {
			if (s != 0) {
				// handle -0.0
				return 0x8000;
			}
			return 0;
		}
		f = (f | 0x00800000) >> (1 - e);
		return s | (f >> 13);
	} else if (e == 0xff - (127 - 15)) {
		if (f == 0) {
			// Inf
			return s | 0x7c00;
		}
		// NAN
		return s | 0x7fff;
	}

	if (e > 30) {
		// Overflow
		return s | 0x7c00;
	}

	return s | (e << 10) | (f >> 13);
}
