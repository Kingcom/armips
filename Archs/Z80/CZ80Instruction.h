#pragma once
#include "z80.h"
#include "z80Opcodes.h"
#include "Commands/CAssemblerCommand.h"
#include "Core/MathParser.h"

typedef struct {
	tZ80RegisterInfo Reg8;
	tZ80RegisterInfo Reg16;
	CExpressionCommandList ImmediateExpression;
	int Immediate;
	int ImmediateBitLen;
	int OriginalImmediate;
} tZ80OpcodeVariables;

class CZ80Instruction: public CAssemblerCommand
{
public:
	CZ80Instruction();
	bool Load(char* Name, char* Params);
	virtual bool Validate(){ return false; };
	virtual void Encode();
	virtual void writeTempData(TempData& tempData) { };
	int GetSize() { return OpcodeSize; };
private:
	bool LoadEncoding(const tZ80Opcode& SourceOpcode, char* Line);
	void FormatInstruction(char* encoding,tZ80OpcodeVariables& Vars, char* dest);
	void WriteInstruction(unsigned short encoding);
	tZ80OpcodeVariables Vars;
	tZ80Opcode Opcode;
	bool NoCheckError;
	bool Loaded;
	int OpcodeSize;
	int RamPos;
};
