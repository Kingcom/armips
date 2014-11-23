#pragma once
#include "Util/CommonClasses.h"
#include "Commands/CAssemblerCommand.h"
#include "ThumbOpcodes.h"
#include "Arm.h"
#include "Core/MathParser.h"

typedef struct {
	tArmRegisterInfo rd;
	tArmRegisterInfo rs;
	tArmRegisterInfo rn;
	tArmRegisterInfo ro;
	CExpressionCommandList ImmediateExpression;
	int Immediate;
	int ImmediateBitLen;
	int OriginalImmediate;
	int rlist;
	char RlistStr[32];
} tThumbOpcodeVariables;

class CThumbInstruction: public CAssemblerCommand
{
public:
	CThumbInstruction();
//	~CThumbInstruction();
	bool Load(char* Name, char* Params);
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
	size_t GetSize() { return OpcodeSize; };
private:
	bool LoadEncoding(const tThumbOpcode& SourceOpcode, char* Line);
	void FormatInstruction(const char* encoding,tThumbOpcodeVariables& Vars, char* dest);
	void WriteInstruction(unsigned short encoding);
	tThumbOpcodeVariables Vars;
	tThumbOpcode Opcode;
	bool NoCheckError;
	bool Loaded;
	size_t OpcodeSize;
	u64 RamPos;
};
