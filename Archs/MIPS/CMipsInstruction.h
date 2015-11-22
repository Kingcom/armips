#pragma once
#include "Commands/CAssemblerCommand.h"
#include "Mips.h"
#include "MipsOpcodes.h"
#include "Core/Expression.h"

enum class MipsRegisterType { Normal, Float, Ps2Cop2, VfpuVector, VfpuMatrix };

enum class MipsImmediateType { None, Immediate5, Immediate8, Immediate16, Immediate20, Immediate26,
	Immediate20_0, ImmediateHalfFloat, Immediate7, Ext, Ins, Cop2BranchType };

struct MipsRegisterValue
{
	MipsRegisterType type;
	std::wstring name;
	int num;
};

struct MipsRegisterData {
	MipsRegisterValue grs;			// general source reg
	MipsRegisterValue grt;			// general target reg
	MipsRegisterValue grd;			// general dest reg
	
	MipsRegisterValue frs;			// float source reg
	MipsRegisterValue frt;			// float target reg
	MipsRegisterValue frd;			// float dest reg
	
	MipsRegisterValue ps2vrs;		// ps2 vector source reg
	MipsRegisterValue ps2vrt;		// ps2 vector target reg
	MipsRegisterValue ps2vrd;		// ps2 vector dest reg

	MipsRegisterValue vrs;			// vfpu source reg
	MipsRegisterValue vrt;			// vfpu target reg
	MipsRegisterValue vrd;			// vfpu dest reg

	void reset()
	{
		grs.num = grt.num = grd.num = -1;
		frs.num = frt.num = frd.num = -1;
		vrs.num = vrt.num = vrd.num = -1;
		ps2vrs.num = ps2vrt.num = ps2vrd.num = -1;
	}
};

struct MipsImmediateData
{
	struct
	{
		MipsImmediateType type;
		Expression expression;
		int value;
		int originalValue;
	} primary;

	struct
	{
		MipsImmediateType type;
		Expression expression;
		int value;
		int originalValue;
	} secondary;

	void reset()
	{
		primary.type = MipsImmediateType::None;
		if (primary.expression.isLoaded())
			primary.expression = Expression();
		
		secondary.type = MipsImmediateType::None;
		if (secondary.expression.isLoaded())
			secondary.expression = Expression();
	}
};

struct MipsOpcodeData
{
	tMipsOpcode opcode;
	int vfpuSize;
	int vectorCondition;

	void reset()
	{
		vfpuSize = vectorCondition = -1;
	}
};

class CMipsInstruction: public CAssemblerCommand
{
public:
	CMipsInstruction(MipsOpcodeData& opcode, MipsImmediateData& immediate, MipsRegisterData& registers);
	~CMipsInstruction();
	virtual bool Validate();
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
private:
	void encodeNormal() const;
	void encodeVfpu() const;
	int floatToHalfFloat(int i);

	bool IgnoreLoadDelay;
	u64 RamPos;
	bool addNop;

	// opcode variables
	MipsOpcodeData opcodeData;
	MipsImmediateData immediateData;
	MipsRegisterData registerData;
};
