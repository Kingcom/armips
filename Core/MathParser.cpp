#include "stdafx.h"
#include "Core/MathParser.h"
#include "Core/Common.h"
#include "Core/FileManager.h"

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
	if (Hex >= 'A' && Hex <= 'f') return true;
	if (Hex >= 'a' && Hex <= 'f') return true;
	return false;
}

bool ConvertToInt(char* str, int defaultrad, int len, int& Result)
{
	int val = 0;
	int r = 0;
	if (len == 0) len = (int) strlen(str);

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


int HexToInt(char* Hex, int length)
{
	int result = 0;
	for (int i = 0; i < length; i++)
	{
		result = (result << 4) | htd(Hex[i]);
	}

	return result;
}

int GetOpcode(char* str, int& ReturnLen, int LastOpcode)
{
	int longestlen = 0;
	int result = EXOP_NONE;

	for (int i = 0; ExpressionOpcodes[i].type != EXOP_NONE; i++)
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
	for (int i = 0; ExpressionOpcodes[i].type != EXOP_NONE; i++)
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
	for (int i = 0; ExpressionOpcodes[i].type != EXOP_NONE; i++)
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

	int InfixPos = 0;
	int InfixLen = strlen(Infix);
	int LastOpcode = EXOP_NONE;
	int OpcodeLen = 0;

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
	int index = 0;

	while (index < Postfix.GetCount())
	{
		int Opcode = GetOpcodeNum(Postfix.GetEntry(index));
		if (Opcode == EXOP_NONE)	// number/variable
		{
			char* str = Postfix.GetEntry(index++);
			if ((str[0] >= '0' && str[0] <= '9') || str[0] == '$' )	// number
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


bool ParsePostfix(CExpressionCommandList& Postfix, CStringList* Errors, int& Result)
{
	IntegerStack Stack;
	unsigned int arg[5];
	char str[255];
	bool Error = false;
	int num = 0;
	int Opcode;
	Label* label;

	while (num < Postfix.GetCount())
	{
		switch (Postfix.GetType(num))
		{
		case EXCOMM_CONST:	// constant
			Stack.Push(Postfix.GetValue(num++));
			break;
		case EXCOMM_VAR:	// label
			label = Postfix.GetLabel(num);
			if (label->isDefined() == false)
			{
				if (Errors != NULL)
				{
					sprintf_s(str,255,"Undefined label \"%ls\"",label->getName().c_str());
					Errors->AddEntry(str);
				}
				Error = true;
				Stack.Push(-1);
				num++;
				break;
			}

			Stack.Push(label->getValue());
			num++;
			break;
		case EXCOMM_RAMPOS:
			Postfix.GetValue(num++);
			Stack.Push(g_fileManager->getVirtualAddress());
			break;
		case EXCOMM_OP:	// opcode
			Opcode = Postfix.GetValue(num++);
			if (Stack.GetCount() < ExpressionCleanOpcodes[Opcode].Arguments) return false;
			for (int l = 0; l < ExpressionCleanOpcodes[Opcode].Arguments; l++)
			{
				arg[l] = Stack.Pop();
			}

			switch (Opcode)
			{
			case EXOP_SIGNPLUS:
				break;
			case EXOP_SIGNMINUS:	// -0
				Stack.Push(0-arg[0]);
				break;
			case EXOP_BITNOT:			// ~b
				Stack.Push(~arg[0]);
				break;
			case EXOP_LOGNOT:			// !b
				Stack.Push(!arg[0]);
				break;
			case EXOP_MUL:			// a*b
				Stack.Push(arg[1]*arg[0]);
				break;
			case EXOP_DIV:			// a/b
				Stack.Push(arg[1]/arg[0]);
				break;
			case EXOP_MOD:			// a%b
				Stack.Push(arg[1]%arg[0]);
				break;
			case EXOP_ADD:			// a+b
				Stack.Push(arg[1]+arg[0]);
				break;
			case EXOP_SUB:			// a-b
				Stack.Push(arg[1]-arg[0]);
				break;
			case EXOP_SHL:			// a<<b
				Stack.Push(arg[1]<<arg[0]);
				break;
			case EXOP_SHR:			// a>>b
				Stack.Push(arg[1]>>arg[0]);
				break;
			case EXOP_GREATEREQUAL:		// a >= b
				Stack.Push(arg[1]>=arg[0]);
				break;
			case EXOP_GREATER:			// a > b
				Stack.Push(arg[1]>arg[0]);
				break;
			case EXOP_LOWEREQUAL:		// a <= b
				Stack.Push(arg[1]<=arg[0]);
				break;
			case EXOP_LOWER:			// a < b
				Stack.Push(arg[1]<arg[0]);
				break;
			case EXOP_EQUAL:		// a == b
				Stack.Push(arg[1]==arg[0]);
				break;
			case EXOP_NOTEQUAL:			// a != b
				Stack.Push(arg[1]!=arg[0]);
				break;
			case EXOP_BITAND:			// a&b
				Stack.Push(arg[1]&arg[0]);
				break;
			case EXOP_XOR:			// a^b
				Stack.Push(arg[1]^arg[0]);
				break;
			case EXOP_BITOR:			// a|b
				Stack.Push(arg[1]|arg[0]);
				break;
			case EXOP_LOGAND:			// a && b
				Stack.Push(arg[1]&&arg[0]);
				break;
			case EXOP_LOGOR:			// a && b
				Stack.Push(arg[1]||arg[0]);
				break;
			case EXOP_TERTIF:			// must not appear
				return false;
			case EXOP_TERTELSE:			// exp ? exp : exp, else comes first!
				if (Postfix.GetValue(num++) != EXOP_TERTIF) return false;
				Stack.Push(arg[2]?arg[1]:arg[0]);
				break;
			}
			break;
		}
	}

	if (Stack.GetCount() != 1) return false;
	Result = Stack.Pop();
	return Error == true ? false : true;
}


CExpressionCommandList::CExpressionCommandList()
{
	Entries = NULL;
	EntryCount = 0;
}

CExpressionCommandList::~CExpressionCommandList()
{
	free(Entries);
}

bool CExpressionCommandList::Load(CStringList &List)
{
	free(Entries);
	Entries = (tExpressionCommandEntry*) malloc(List.GetCount() * sizeof(tExpressionCommandEntry));
	EntryCount = List.GetCount();

	for (int i = 0; i < List.GetCount(); i++)
	{
		char* str = List.GetEntry(i);
		int num = GetOpcodeNum(str);

		if (num != EXOP_NONE)	// operator
		{
			Entries[i].command = EXCOMM_OP;
			Entries[i].num = num;
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
			Label* label = Global.symbolTable.getLabel(convertUtf8ToWString(str),Global.FileInfo.FileNum,Global.Section);
			if (label == NULL)
			{
				Logger::printError(Logger::Error,L"Invalid label name \"%S\"",str);
				return false;
			}

			Entries[i].command = EXCOMM_VAR;
			Entries[i].label = label;
		}
	}


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
			for (int l = 0; l < List.GetCount(); l++)
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
