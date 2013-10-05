#pragma once
#include "Commands/CAssemblerCommand.h"
#include "Mips.h"
#include "MipsOpcodes.h"
#include "Core/MathParser.h"

typedef enum eMipsImmediateType { MIPS_NOIMMEDIATE, MIPS_IMMEDIATE5,
	MIPS_IMMEDIATE16, MIPS_IMMEDIATE20, MIPS_IMMEDIATE26 };

typedef struct {
	tMipsRegisterInfo rs;			// source reg
	tMipsRegisterInfo rt;			// target reg
	tMipsRegisterInfo rd;			// dest reg
	eMipsImmediateType ImmediateType;
	CExpressionCommandList ImmediateExpression;
	int Immediate;
	int OriginalImmediate;
} tMipsOpcodeVariables;

class CMipsInstruction: public CAssemblerCommand
{
public:
	CMipsInstruction();
	~CMipsInstruction();
	bool Load(char* Name, char* Params);
	virtual bool Validate();
	virtual void Encode();
	virtual void WriteTempData(FILE*& Output);
private:
	bool LoadEncoding(const tMipsOpcode& SourceOpcode, char* Line);
	void FormatInstruction(char* encoding,tMipsOpcodeVariables& Vars, char* dest);
	void WriteInstruction(unsigned int encoding);
	tMipsOpcode Opcode;
	bool IgnoreLoadDelay;
	bool NoCheckError;
	bool Loaded;
	tMipsOpcodeVariables Vars;
	int RamPos;
	CMipsInstruction* SubInstruction;
	bool SubInstructionEnabled;
};
