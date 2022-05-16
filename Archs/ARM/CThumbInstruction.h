#pragma once

#include "Archs/ARM/Arm.h"
#include "Archs/ARM/ThumbOpcodes.h"
#include "Core/Expression.h"

struct ThumbOpcodeVariables {
	ArmRegisterValue rd;
	ArmRegisterValue rs;
	ArmRegisterValue rn;
	ArmRegisterValue ro;
	Expression ImmediateExpression;
	int Immediate;
	int ImmediateBitLen;
	int OriginalImmediate;
	int rlist;
	unsigned int NewEncoding;
	bool UseNewEncoding;
	char RlistStr[32];
} ;

class CThumbInstruction: public ArmOpcodeCommand
{
public:
	CThumbInstruction(const tThumbOpcode& sourceOpcode, ThumbOpcodeVariables& vars);
//	~CThumbInstruction();
	bool Load(char* Name, char* Params);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	size_t GetSize() { return OpcodeSize; };
	void setPoolAddress(int64_t address) override;
private:
	void FormatInstruction(const char* encoding, char* dest) const;
	void WriteInstruction(unsigned short encoding) const;
	ThumbOpcodeVariables Vars;
	tThumbOpcode Opcode;
	size_t OpcodeSize;
	int64_t RamPos;
};
