#pragma once
#include "Util/CommonClasses.h"

typedef struct {
	char Name[3];
	unsigned char Priority;
	unsigned char len;
	unsigned char type;
	bool sign;
	bool IgnoreCheck;
} tExpressionOpcode;

typedef struct {
	char Name[3];
	unsigned char Arguments;
} tExpressionCleanOpcode;

#define EXOP_NONE			0x00
#define EXOP_BRACKETL		0x01
#define EXOP_BRACKETR		0x02
#define EXOP_SIGNPLUS		0x03
#define EXOP_SIGNMINUS		0x04
#define EXOP_BITNOT			0x05
#define EXOP_LOGNOT			0x06
#define EXOP_MUL			0x07
#define EXOP_DIV			0x08
#define EXOP_MOD			0x09
#define EXOP_ADD			0x0A
#define EXOP_SUB			0x0B
#define EXOP_SHL			0x0C
#define EXOP_SHR			0x0D
#define EXOP_GREATEREQUAL	0x0E
#define EXOP_GREATER		0x0F
#define EXOP_LOWEREQUAL		0x10
#define EXOP_LOWER			0x11
#define EXOP_EQUAL			0x12
#define EXOP_NOTEQUAL		0x13
#define EXOP_BITAND			0x14
#define EXOP_XOR			0x15
#define EXOP_BITOR			0x16
#define EXOP_LOGAND			0x17
#define EXOP_LOGOR			0x18
#define EXOP_TERTIF			0x19
#define EXOP_TERTELSE		0x1A
#define EXOP_NUMBER			0x1B		// dummy

#define EXOP_COUNT			0x1B


class CExpressionOpcodeStack
{
public:
	CExpressionOpcodeStack() { pos = -1; };
	void Push(const char str[3]) { strcpy(Opcodes[++pos],str); };
	void Pop(char* dest) { strcpy(dest,Opcodes[pos--]); };
	bool IsEmpty() { return pos == -1 ? true : false; };
private:
	char Opcodes[512][3];
	int pos;
};



typedef enum eExpressionCommand { EXCOMM_OP, EXCOMM_CONST, EXCOMM_VAR, EXCOMM_RAMPOS };

class Label;

typedef struct {
	eExpressionCommand command;
	union
	{
		int num;
		Label* label;
	};
} tExpressionCommandEntry;

class CExpressionCommandList
{
public:
	CExpressionCommandList();
	~CExpressionCommandList();
	bool Load(CStringList &List);
	eExpressionCommand GetType(int i) { return Entries[i].command; };
	unsigned int GetValue(int i) { return Entries[i].num; };
	Label* GetLabel(int i) { return Entries[i].label; };
	int GetCount() { return EntryCount; };
private:
	tExpressionCommandEntry* Entries;
	int EntryCount;
};

bool IsHex(char Hex);
inline int htd(char Hex);
bool ConvertToInt(char* str, int defaultrad, int len, int& Result);
int HexToInt(char* Hex, int length);
bool ConvertInfixToPostfix(char* Infix, CStringList& Postfix);
bool CheckPostfix(CStringList& Postfix, bool AllowLabels);
bool ParsePostfix(CExpressionCommandList& Postfix, CStringList* Errors, int& Result);
bool ConvertExpression(char* exp, int& Result);

