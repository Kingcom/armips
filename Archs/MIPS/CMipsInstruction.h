#pragma once
#include "Commands/CAssemblerCommand.h"
#include "Mips.h"
#include "MipsOpcodes.h"
#include "Core/MathParser.h"

enum class MipsImmediateType { None, Immediate5, Immediate8, Immediate16, Immediate20, Immediate26,
	Immediate20_0, ImmediateHalfFloat, Immediate7 };
enum class MipsSecondaryImmediateType { None, Ext, Ins, Cop2BranchType };

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
	
	MipsRegisterInfo ps2vrs;		// ps2 vector source reg
	MipsRegisterInfo ps2vrt;		// ps2 vector target reg
	MipsRegisterInfo ps2vrd;		// ps2 vector dest reg

	MipsVFPURegister vrs;			// vfpu source reg
	MipsVFPURegister vrt;			// vfpu target reg
	MipsVFPURegister vrd;			// vfpu dest reg

	void reset()
	{
		grs.num = grt.num = grd.num = -1;
		frs.num = frt.num = frd.num = -1;
		vrs.num = vrt.num = vrd.num = -1;
		ps2vrs.num = ps2vrt.num = ps2vrd.num = -1;
	}
};


class CMipsInstruction: public CAssemblerCommand
{
public:
	CMipsInstruction();
	~CMipsInstruction();
	bool Load(const char* Name, const char* Params);
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
private:
	void encodeNormal();
	void encodeVfpu();
	bool parseOpcode(const tMipsOpcode& SourceOpcode, const char* Line);
	bool LoadEncoding(const tMipsOpcode& SourceOpcode, const char* Line);
	void setOmittedRegisters();
	int formatOpcodeName(char* dest);
	void formatParameters(char* dest);

	tMipsOpcode Opcode;
	bool IgnoreLoadDelay;
	bool NoCheckError;
	bool Loaded;
	u64 RamPos;
	CMipsInstruction* subInstruction;

	// opcode variables
	MipsOpcodeRegisters registers;
	MipsImmediateType immediateType;
	MipsImmediate immediate;
	int vfpuSize;
	int vectorCondition;
	
	bool hasFixedSecondaryImmediate;
	MipsSecondaryImmediateType secondaryImmediateType;
	MipsImmediate secondaryImmediate;
};
