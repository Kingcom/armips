#include "stdafx.h"
#include "Core/MathParser.h"
#include "Core/Common.h"
#include "Core/FileManager.h"

#ifdef _WIN32
#define snprintf sprintf_s
#endif

const tExpressionCleanOpcode ExpressionCleanOpcodes[EXOP_COUNT] = {
	{"", 0},	{"(",0},	{")",0},
	{"|+",1},	{"|-",1},	{"~",1},
	{"!",1},	{"*",2},	{"/",2},
	{"%",2},	{"+",2},	{"-",2},
	{"<<",2},	{">>",2},	{">=",2},
	{">",2},	{"<=",2},	{"<",2},
	{"==",2},	{"!=",2},	{"&",2},
	{"^",2},	{"|",2},	{"&&",2},
	{"||",2},	{"?",0},	{":",3}
};

const tExpressionOpcode ExpressionOpcodes[] = {
	{ "(",	13,	1,	EXOP_BRACKETL,		false,	false },
	{ ")",	13,	1,	EXOP_BRACKETR,		false,	false },
	{ "+",	12,	1,	EXOP_SIGNPLUS,		true,	false },
	{ "|+",	12,	1,	EXOP_SIGNPLUS,		true,	true },
	{ "-",	12,	1,	EXOP_SIGNMINUS,		true,	false },
	{ "|-",	12,	1,	EXOP_SIGNMINUS,		true,	true },
	{ "~",	12,	1,	EXOP_BITNOT,		false,	false }, 
	{ "!",	12,	1,	EXOP_LOGNOT,		false,	false }, 
	{ "*",	11,	1,	EXOP_MUL,			false,	false },
	{ "/",	11,	1,	EXOP_DIV,			false,	false },
	{ "%",	11,	1,	EXOP_MOD,			false,	false },
	{ "+",	10,	1,	EXOP_ADD,			false,	false },
	{ "-",	10,	1,	EXOP_SUB,			false,	false },
	{ "<<",	9,	2,	EXOP_SHL,			false,	false },
	{ ">>",	9,	2,	EXOP_SHR,			false,	false },
	{ ">=",	8,	2,	EXOP_GREATEREQUAL,	false,	false },
	{ ">",	8,	1,	EXOP_GREATER,		false,	false },
	{ "<=",	8,	2,	EXOP_LOWEREQUAL,	false,	false },
	{ "<",	8,	1,	EXOP_LOWER,			false,	false },
	{ "==",	7,	2,	EXOP_EQUAL,			false,	false },
	{ "!=",	7,	2,	EXOP_NOTEQUAL,		false,	false },
	{ "&",	6,	1,	EXOP_BITAND,		false,	false },
	{ "^",	5,	1,	EXOP_XOR,			false,	false },
	{ "|",	4,	1,	EXOP_BITOR,			false,	false },
	{ "&&",	3,	2,	EXOP_LOGAND,		false,	false },
	{ "||",	2,	2,	EXOP_LOGOR,			false,	false },
	{ ":",	1,	1,	EXOP_TERTELSE,		false,	false },
	{ "?",	0,	1,	EXOP_TERTIF,		false,	false },
	{ "",	0,	0,	EXOP_NONE,			false,	false }
};

MathExpression::MathExpression()
{
	loaded = false;
	checked = false;
}

MathExpression::MathExpression(MathExpression& other)
{
	*this = other;
}

MathExpression& MathExpression::operator=(MathExpression& other)
{
	loaded = false;
	checked = false;

	if (other.loaded)
		init(other.originalText,other.allowLabels);
	if (other.checked)
		check();

	return *this;
}

bool MathExpression::init(const std::wstring& infix, bool allowLabels)
{
	this->allowLabels = allowLabels;
	originalText = infix;

	std::string utf8 = convertWStringToUtf8(infix);
	loaded = ConvertInfixToPostfix((char*)utf8.c_str(),postfix);
	return loaded;
}

bool MathExpression::check()
{
	if (CheckPostfix(postfix,allowLabels) == false)
	{
		Logger::printError(Logger::Error,L"Invalid expression \"%s\"",originalText);
		return false;
	}

	if (expList.Load(postfix) == false)
	{
		Logger::printError(Logger::Error,L"Invalid expression \"%s\"",originalText);
		return false;
	}
	
	checked = true;
	return true;
}

bool MathExpression::evaluate(int& dest, bool queue)
{
	CStringList List;
	if (ParsePostfix(expList,&List,dest) == false)
	{
		if (List.GetCount() == 0)
		{
			if (queue)
				Logger::queueError(Logger::Error,L"Invalid expression");
			else
				Logger::printError(Logger::Error,L"Invalid expression");
		} else {
			for (size_t l = 0; l < List.GetCount(); l++)
			{
				if (queue)
					Logger::queueError(Logger::Error,convertUtf8ToWString(List.GetEntry(l)));
				else
					Logger::printError(Logger::Error,convertUtf8ToWString(List.GetEntry(l)));
			}
		}
		return false;
	}

	return true;
}

inline int htd(char Hex)
{
	switch(Hex)
	{
	case '0': case '1': case '2':
	case '3': case '4': case '5':
	case '6': case '7': case '8':
	case '9':
		return (int) Hex-0x30;
	case 'A': case 'B': case 'C':
	case 'D': case 'E': case 'F':
		return (int) Hex-0x37;
	case 'a': case 'b': case 'c':
	case 'd': case 'e': case 'f':
		return (int) Hex-0x57;
	}
	printf("Error: %c is no hex character\n",Hex);
	return -1;
}

bool IsHex(char Hex)
{
	if (Hex >= '0' && Hex <= '9') return true;
	if (Hex >= 'A' && Hex <= 'F') return true;
	if (Hex >= 'a' && Hex <= 'f') return true;
	return false;
}

bool ConvertToInt(char* str, int defaultrad, size_t len, int& Result)
{
	int val = 0;
	int r = 0;
	if (len == 0) len = strlen(str);

	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
	{
		r = 16;
		str+=2;
		len-=2;
	} else if (str[0] == '$')
	{
		r = 16;
		str++;
		len--;
	} else if (str[0] == '0' && str[1] == 'o')
	{
		r = 8;
		str+=2;
		len-=2;
	} else {
		if (!(str[0] >= '0' && str[0] <= '9')) return false;

		if (str[len-1] == 'b')
		{
			r = 2;
			len--;
		} else if (str[len-1] == 'o')
		{
			r = 8;
			len--;
		} else if (str[len-1] == 'h')
		{
			r = 16;
			len--;
		} else {
			r = defaultrad;
		}
	}

	switch (r)
	{
	case 2: // bin
		while (len--)
		{
			if (*str != '0' && *str != '1') return false;
			val = val << 1;
			if (*str++ == '1')
			{
				val++;
			}
		}
		break;
	case 8: // oct
		while (len--)
		{
			if (*str < '0' || *str > '7') return false;
			val = val << 3;
			val+=(*str++-'0');
		}
		break;
	case 10: // dec
		while (len--)
		{
			if (*str < '0' || *str > '9') return false;
			val = val * 10;
			val += (*str++ - '0');
		}
		break;
	case 16: // hex
		while (len--)
		{
			if ((*str < '0' || *str > '9') && (*str < 'A' || *str > 'F') && (*str < 'a' || *str > 'f')) return false;
			int l = htd(*str++);
			if (l >= 0)
			{
				val = val << 4;
				val += l;
			} else {
				return false;
			}
		}
		break;
	default:
		return false;
	}
	Result = val;
	return true;
}


int HexToInt(char* Hex, size_t length)
{
	int result = 0;
	for (size_t i = 0; i < length; i++)
	{
		result = (result << 4) | htd(Hex[i]);
	}

	return result;
}

// Parse only a float, and return as float bits.
static bool parseFloat(const char *str, size_t len, int& result)
{
	if (!(str[0] >= '0' && str[0] <= '9'))
		return false;

	bool foundDecimal = false;
	for (size_t i = 0; i < len; ++i)
	{
		if (str[i] == '.')
		{
			if (foundDecimal)
				return false;
			foundDecimal = true;
			continue;
		}
		if (str[i] < '0' || str[i] > '9')
		return false;
	}

	float f = (float)atof(str);
	memcpy(&result, &f, sizeof(result));
	return foundDecimal;
}

int GetOpcode(char* str, size_t& ReturnLen, int LastOpcode)
{
	int longestlen = 0;
	int result = EXOP_NONE;

	for (size_t i = 0; ExpressionOpcodes[i].type != EXOP_NONE; i++)
	{
		if (ExpressionOpcodes[i].IgnoreCheck == true) continue;
		if (ExpressionOpcodes[i].sign == true &&
			(LastOpcode == EXOP_NUMBER || LastOpcode == EXOP_BRACKETR)) continue;

		int len = ExpressionOpcodes[i].len;
		if (len > longestlen)
		{
			if (strncmp(ExpressionOpcodes[i].Name,str,len) == 0)
			{
				result = ExpressionOpcodes[i].type;
				longestlen = len;
			}
		}
	}

	ReturnLen = longestlen;
	return result;
}

int GetOpcodePriority(char* opcode)
{
	for (size_t i = 0; ExpressionOpcodes[i].type != EXOP_NONE; i++)
	{
		if (ExpressionOpcodes[i].sign == true && ExpressionOpcodes[i].IgnoreCheck == false) continue;
		if (strcmp(opcode,ExpressionOpcodes[i].Name) == 0)
		{
			return ExpressionOpcodes[i].Priority;
		}
	}
	return -1;
}

int GetOpcodeNum(const char* opcode)
{
	if (opcode == NULL) return EXOP_NONE;
	for (size_t i = 0; ExpressionOpcodes[i].type != EXOP_NONE; i++)
	{
		if (ExpressionOpcodes[i].sign == true && ExpressionOpcodes[i].IgnoreCheck == false) continue;
		if (strcmp(opcode,ExpressionOpcodes[i].Name) == 0)
		{
			return ExpressionOpcodes[i].type;
		}
	}
	return EXOP_NONE;
}


inline bool IsAlphaNum(char c)
{
	if ((c >= '0' && c <= '9') ||
		(c >= 'A' && c <= 'Z') ||
		(c >= 'a' && c <= 'z') ||
		c == '@' || c == '_' || c == '.' || c == '$')
	{
		return true;
	} else {
		return false;
	}
}

bool ConvertInfixToPostfix(char* Infix, CStringList& Postfix)
{
	CExpressionOpcodeStack OpcodeStack;
	char Buffer[512];
	Postfix.Clear();

	size_t InfixPos = 0;
	size_t InfixLen = strlen(Infix);
	int LastOpcode = EXOP_NONE;
	size_t OpcodeLen = 0;

	while (InfixPos < InfixLen)
	{
		while (Infix[InfixPos] == ' ' || Infix[InfixPos] == '	') InfixPos++;
		int Opcode = GetOpcode(&Infix[InfixPos],OpcodeLen,LastOpcode);
		if (Opcode == EXOP_NONE)	// number/variable
		{
			int BufferPos = 0;

			if (Infix[InfixPos] == '\'' && Infix[InfixPos+2] == '\'')
			{
				Buffer[BufferPos++] = Infix[InfixPos++];
				Buffer[BufferPos++] = Infix[InfixPos++];
				Buffer[BufferPos++] = Infix[InfixPos++];
				Buffer[BufferPos] = 0;
				Postfix.AddEntry(Buffer);
				LastOpcode = EXOP_NUMBER;
				continue;
			}

			if (IsAlphaNum(Infix[InfixPos]) == false)
			{
				return false;
			}

			while (IsAlphaNum(Infix[InfixPos]))
			{
				Buffer[BufferPos++] = Infix[InfixPos++];
			}
			Buffer[BufferPos] = 0;
			Postfix.AddEntry(Buffer);
			LastOpcode = EXOP_NUMBER;
		} else {	// opcode
			switch (Opcode)
			{
			case EXOP_BRACKETL:
				OpcodeStack.Push(ExpressionCleanOpcodes[Opcode].Name);
				break;
			case EXOP_BRACKETR:
				while (true)
				{
					if (OpcodeStack.IsEmpty()) return false;
					OpcodeStack.Pop(Buffer);
					if (strcmp(Buffer,"(") == 0) break;
					Postfix.AddEntry(Buffer);
				}
				break;
			default:
				if (OpcodeStack.IsEmpty() == false)
				{
					int CurrentPriority = GetOpcodePriority((char*)ExpressionCleanOpcodes[Opcode].Name);
					while (!OpcodeStack.IsEmpty())
					{
						OpcodeStack.Pop(Buffer);
						if (strcmp(Buffer,"(") == 0)
						{
							OpcodeStack.Push(Buffer);
							break;
						}
						if (GetOpcodePriority(Buffer) >= CurrentPriority)
						{
							Postfix.AddEntry(Buffer);
						} else {
							OpcodeStack.Push(Buffer);
							break;
						}
					}
				}
				OpcodeStack.Push(ExpressionCleanOpcodes[Opcode].Name);
				break;
			}
			InfixPos += OpcodeLen;
			LastOpcode = Opcode;
		}
	}

	while (!OpcodeStack.IsEmpty())
	{
		OpcodeStack.Pop(Buffer);
		if (strcmp(Buffer,"(") == 0) return false;	// open paranthesis without closing one
		Postfix.AddEntry(Buffer);
	}
	return true;
}

bool CheckPostfix(CStringList& Postfix, bool AllowLabels)
{
	int StackPos = 0;
	int num;
	size_t index = 0;

	while (index < Postfix.GetCount())
	{
		int Opcode = GetOpcodeNum(Postfix.GetEntry(index));
		if (Opcode == EXOP_NONE)	// number/variable
		{
			char* str = Postfix.GetEntry(index++);
			if (parseFloat(str,strlen(str),num))
			{
				StackPos++;
			} else if ((str[0] >= '0' && str[0] <= '9') || str[0] == '$' )	// number
			{
				if (ConvertToInt(str,Global.Radix,0,num) == false) return false;
				StackPos++;
			} else if (str[0] == '\'' && str[2] == '\'' && str[3] == 0)
			{
				StackPos++;
			} else if (strcmp(str,".") == 0)
			{
				if (AllowLabels == false) return false;
				StackPos++;
			} else {
				if (AllowLabels == false) return false;
				if (Global.symbolTable.isValidSymbolName(convertUtf8ToWString(str)) == false) return false;
				StackPos++;
			}
		} else {
			if (Opcode == EXOP_TERTELSE)
			{
				if (GetOpcodeNum(Postfix.GetEntry(++index)) != EXOP_TERTIF) return false;
			}
			if (StackPos < ExpressionCleanOpcodes[Opcode].Arguments) return false;
			StackPos = StackPos - ExpressionCleanOpcodes[Opcode].Arguments + 1;
			index++;
		}
	}
	if (StackPos != 1) return false;
	return true;
}

inline int f2i(float f)
{
	union
	{
		float f;
		int i;
	} u;

	u.f = f;
	return u.i;
}

inline float i2f(int i)
{
	union
	{
		float f;
		int i;
	} u;

	u.i = i;
	return u.f;
}

bool ParsePostfix(CExpressionCommandList& Postfix, CStringList* Errors, int& Result)
{
	IntegerStack Stack;
	IntegerStack TypeStack;

	unsigned int arg[5];
	float fArg[5];
	char str[255];
	bool Error = false;
	size_t num = 0;
	int Opcode;
	Label* label;

	while (num < Postfix.GetCount())
	{
		switch (Postfix.GetType(num))
		{
		case EXCOMM_CONST:	// constant
			Stack.push(Postfix.GetValue(num++));
			TypeStack.push(EXCOMM_CONST);
			break;
		case EXCOMM_FLOAT:	// float constant
			Stack.push(Postfix.GetValue(num++));
			TypeStack.push(EXCOMM_FLOAT);
			break;
		case EXCOMM_VAR:	// label
			if (Postfix.LabelExists(num) == false)
			{
				if (Errors != NULL)
				{
					snprintf(str,255,"Undefined label \"%ls\"",Postfix.GetLabelName(num).c_str());
					Errors->AddEntry(str);
				}
				Error = true;
				Stack.push(-1);
				TypeStack.push(EXCOMM_CONST);
				num++;
				break;
			}
			label = Postfix.GetLabel(num);
			Stack.push((unsigned int)label->getValue());
			TypeStack.push(EXCOMM_CONST);
			num++;
			break;
		case EXCOMM_RAMPOS:
			Postfix.GetValue(num++);
			Stack.push((unsigned int)g_fileManager->getVirtualAddress());
			TypeStack.push(EXCOMM_CONST);
			break;
		case EXCOMM_OP:	// opcode
			Opcode = Postfix.GetValue(num++);
			if (Stack.size() < ExpressionCleanOpcodes[Opcode].Arguments) return false;

			int type = EXCOMM_CONST;
			for (int l = 0; l < ExpressionCleanOpcodes[Opcode].Arguments; l++)
			{
				int value = Stack.pop();

				int argType = TypeStack.pop();
				if (type == EXCOMM_CONST)
					type = argType;

				switch (argType)
				{
				case EXCOMM_CONST:
					arg[l] = value;
					fArg[l] = (float) value;
					break;
				case EXCOMM_FLOAT:
					arg[l] = value;
					fArg[l] = i2f(value);
					break;
				}
			}

			bool useFloat = type == EXCOMM_FLOAT;
			int resultType = EXCOMM_CONST;
			switch (Opcode)
			{
			case EXOP_SIGNPLUS:
				Stack.push(arg[0]);
				resultType = type;
				break;
			case EXOP_SIGNMINUS:	// -0
				if (useFloat)
					Stack.push(f2i(0.0f-fArg[0]));
				else
					Stack.push(0-arg[0]);
				resultType = type;
				break;
			case EXOP_BITNOT:			// ~b
				Stack.push(~arg[0]);
				break;
			case EXOP_LOGNOT:			// !b
				Stack.push(!arg[0]);
				break;
			case EXOP_MUL:			// a*b
				if (useFloat)
					Stack.push(f2i(fArg[1]*fArg[0]));
				else
					Stack.push(arg[1]*arg[0]);
				resultType = type;
				break;
			case EXOP_DIV:			// a/b
				if (useFloat)
					Stack.push(f2i(fArg[1]/fArg[0]));
				else
					Stack.push(arg[1]/arg[0]);
				resultType = type;
				break;
			case EXOP_MOD:			// a%b
				Stack.push(arg[1]%arg[0]);
				break;
			case EXOP_ADD:			// a+b
				if (useFloat)
					Stack.push(f2i(fArg[1]+fArg[0]));
				else
					Stack.push(arg[1]+arg[0]);
				resultType = type;
				break;
			case EXOP_SUB:			// a-b
				if (useFloat)
					Stack.push(f2i(fArg[1]-fArg[0]));
				else
					Stack.push(arg[1]-arg[0]);
				resultType = type;
				break;
			case EXOP_SHL:			// a<<b
				Stack.push(arg[1]<<arg[0]);
				break;
			case EXOP_SHR:			// a>>b
				Stack.push(arg[1]>>arg[0]);
				break;
			case EXOP_GREATEREQUAL:		// a >= b
				if (useFloat)
					Stack.push(fArg[1]>=fArg[0]);
				else
					Stack.push(arg[1]>=arg[0]);
				break;
			case EXOP_GREATER:			// a > b
				if (useFloat)
					Stack.push(fArg[1]>fArg[0]);
				else
					Stack.push(arg[1]>arg[0]);
				break;
			case EXOP_LOWEREQUAL:		// a <= b
				if (useFloat)
					Stack.push(fArg[1]<=fArg[0]);
				else
					Stack.push(arg[1]<=arg[0]);
				break;
			case EXOP_LOWER:			// a < b
				if (useFloat)
					Stack.push(fArg[1]<fArg[0]);
				else
					Stack.push(arg[1]<arg[0]);
				break;
			case EXOP_EQUAL:		// a == b
				Stack.push(arg[1]==arg[0]);
				break;
			case EXOP_NOTEQUAL:			// a != b
				Stack.push(arg[1]!=arg[0]);
				break;
			case EXOP_BITAND:			// a&b
				Stack.push(arg[1]&arg[0]);
				break;
			case EXOP_XOR:			// a^b
				Stack.push(arg[1]^arg[0]);
				break;
			case EXOP_BITOR:			// a|b
				Stack.push(arg[1]|arg[0]);
				break;
			case EXOP_LOGAND:			// a && b
				Stack.push(arg[1]&&arg[0]);
				break;
			case EXOP_LOGOR:			// a || b
				Stack.push(arg[1]||arg[0]);
				break;
			case EXOP_TERTIF:			// must not appear
				return false;
			case EXOP_TERTELSE:			// exp ? exp : exp, else comes first!
				if (Postfix.GetValue(num++) != EXOP_TERTIF) return false;
				Stack.push(arg[2]?arg[1]:arg[0]);
				break;
			}

			TypeStack.push(resultType);
			break;
		}
	}

	if (Stack.size() != 1) return false;
	Result = Stack.pop();
	return Error == true ? false : true;
}


CExpressionCommandList::CExpressionCommandList()
{
	Entries = NULL;
	EntryCount = 0;
	initialized = false;
}

CExpressionCommandList::~CExpressionCommandList()
{
	delete[] Entries;
}

bool CExpressionCommandList::Load(CStringList &List)
{
	if (Entries != NULL)
		delete[] Entries;
	Entries = new tExpressionCommandEntry[List.GetCount()];
	EntryCount = List.GetCount();
	initialized = false;

	for (size_t i = 0; i < List.GetCount(); i++)
	{
		char* str = List.GetEntry(i);
		int num = GetOpcodeNum(str);

		if (num != EXOP_NONE)	// operator
		{
			Entries[i].command = EXCOMM_OP;
			Entries[i].num = num;
		} else if (parseFloat(str,strlen(str),Entries[i].num))
		{
			Entries[i].command = EXCOMM_FLOAT;
		} else if ((str[0] >= '0' && str[0] <= '9') || str[0] == '$')	// constant
		{
			Entries[i].command = EXCOMM_CONST;
			if (ConvertToInt(str,Global.Radix,0,Entries[i].num) == false)
			{
				Logger::printError(Logger::Error,L"Invalid number \"%S\"",str);
				return false;
			}
		} else if (str[0] == '\'' && str[2] == '\'' && str[3] == 0)	// ascii
		{
			Entries[i].command = EXCOMM_CONST;
			Entries[i].num = str[1];
		} else if (strcmp(str,".") == 0)	// rampos
		{
			Entries[i].command = EXCOMM_RAMPOS;
			Entries[i].num = 0;
		} else {	// variable
			if (Global.symbolTable.isValidSymbolName(convertUtf8ToWString(str)) == false)
			{
				Logger::printError(Logger::Error,L"Invalid label name \"%S\"",str);
				return false;
			}

			Entries[i].command = EXCOMM_VAR;
			Entries[i].label = convertUtf8ToWString(str);
		}
	}

	initialized = true;
	return true;
}

bool ConvertExpression(const std::wstring& exp, int& Result)
{
	CStringList List;
	CExpressionCommandList ExpList;
	if (ConvertInfixToPostfix((char*)convertWStringToUtf8(exp).c_str(),List) == false) return false;
	if (CheckPostfix(List,false) == false) return false;
	if (ExpList.Load(List) == false) return false;
	if (ParsePostfix(ExpList,NULL,Result) == false) return false;
	return true;
}

bool initExpression(CExpressionCommandList& dest, const std::wstring& source, bool queue)
{
	CStringList List;
	
	std::string utf8 = convertWStringToUtf8(source);
	char* src = (char*) utf8.c_str();

	if (ConvertInfixToPostfix(src,List) == false)
	{
		if (queue)
			Logger::queueError(Logger::Error,L"Invalid expression \"%S\"",src);
		else
			Logger::printError(Logger::Error,L"Invalid expression \"%S\"",src);
		return false;
	}
	
	if (CheckPostfix(List,true) == false)
	{
		if (queue)
			Logger::queueError(Logger::Error,L"Invalid expression \"%S\"",src);
		else
			Logger::printError(Logger::Error,L"Invalid expression \"%S\"",src);
		return false;
	}
	
	return dest.Load(List);
}

bool evalExpression(CExpressionCommandList& exp, int& dest, bool queue)
{
	CStringList List;
	if (ParsePostfix(exp,&List,dest) == false)
	{
		if (List.GetCount() == 0)
		{
			if (queue)
				Logger::queueError(Logger::Error,L"Invalid expression");
			else
				Logger::printError(Logger::Error,L"Invalid expression");
		} else {
			for (size_t l = 0; l < List.GetCount(); l++)
			{
				if (queue)
					Logger::queueError(Logger::Error,convertUtf8ToWString(List.GetEntry(l)));
				else
					Logger::printError(Logger::Error,convertUtf8ToWString(List.GetEntry(l)));
			}
		}
		return false;
	}

	return true;
}
