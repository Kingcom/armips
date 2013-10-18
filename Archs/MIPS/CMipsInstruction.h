#pragma once
#include "Commands/CAssemblerCommand.h"
#include "Mips.h"
#include "MipsOpcodes.h"
#include "Core/MathParser.h"

typedef enum MipsImmediateType { MIPS_NOIMMEDIATE, MIPS_IMMEDIATE5,
	MIPS_IMMEDIATE16, MIPS_IMMEDIATE20, MIPS_IMMEDIATE26 };

struct MipsImmediate
{
	MathExpression expression;
	int value;
	int originalValue;
};

struct MipsOpcodeVariables {
	tMipsRegisterInfo rs;			// source reg
	tMipsRegisterInfo rt;			// target reg
	tMipsRegisterInfo rd;			// dest reg

};

class CMipsInstruction: public CAssemblerCommand
{
public:
	CMipsInstruction();
	~CMipsInstruction();
	bool Load(char* Name, char* Params);
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
private:
	bool LoadEncoding(const tMipsOpcode& SourceOpcode, char* Line);
	void setOmittedRegisters();
	void FormatInstruction(char* encoding,MipsOpcodeVariables& Vars, char* dest);

	tMipsOpcode Opcode;
	bool IgnoreLoadDelay;
	bool NoCheckError;
	bool Loaded;
	int RamPos;
	CMipsInstruction* subInstruction;

	// opcode variables
	MipsOpcodeVariables vars;
	MipsImmediateType immediateType;
	MipsImmediate immediate;
};
