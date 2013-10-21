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

struct MipsOpcodeRegisters {
	MipsRegisterInfo grs;			// general source reg
	MipsRegisterInfo grt;			// general target reg
	MipsRegisterInfo grd;			// general dest reg
	
	MipsRegisterInfo frs;			// float source reg
	MipsRegisterInfo frt;			// float target reg
	MipsRegisterInfo frd;			// float dest reg

	MipsVFPURegister vrs;			// vfpu source reg
	MipsVFPURegister vrt;			// vfpu target reg
	MipsVFPURegister vrd;			// vfpu dest reg

	void reset()
	{
		grs.num = grt.num = grd.num = -1;
		frs.num = frt.num = frd.num = -1;
		vrs.num = vrt.num = vrd.num = -1;
	}
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
	void encodeNormal();
	void encodeVfpu();
	bool parseOpcode(const tMipsOpcode& SourceOpcode, char* Line);
	bool LoadEncoding(const tMipsOpcode& SourceOpcode, char* Line);
	void setOmittedRegisters();
	int formatOpcodeName(char* dest);
	void formatParameters(char* dest);

	tMipsOpcode Opcode;
	bool IgnoreLoadDelay;
	bool NoCheckError;
	bool Loaded;
	int RamPos;
	CMipsInstruction* subInstruction;

	// opcode variables
	MipsOpcodeRegisters registers;
	MipsImmediateType immediateType;
	MipsImmediate immediate;
	int vfpuSize;
};
