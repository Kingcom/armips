#pragma once

#include "Archs/SuperH/ShOpcodes.h"
#include "Commands/CAssemblerCommand.h"
#include "Core/Expression.h"
#include "Core/Types.h"

enum class ShRegisterType
{
	Normal,
};

enum class ShImmediateType
{
	None,
	Immediate4,
	Immediate8,
	Immediate12,
};

struct ShRegisterValue
{
	ShRegisterType type;
	Identifier name{};
	int num;
};

struct ShRegisterData {
	ShRegisterValue grs;			// general source reg
	ShRegisterValue grt;			// general target reg

	void reset()
	{
		grs.num = grt.num = -1;
	}
};

struct ShImmediateData
{
	struct
	{
		ShImmediateType type;
		Expression expression;
		int value;
		int originalValue;
	} primary;

	void reset()
	{
		primary.type = ShImmediateType::None;
		if (primary.expression.isLoaded())
			primary.expression = Expression();
	}
};

struct ShOpcodeData
{
	tShOpcode opcode;

	void reset()
	{
	}
};

class CShInstruction: public CAssemblerCommand
{
public:
	CShInstruction(ShOpcodeData& opcode, ShImmediateData& immediate, ShRegisterData& registers);
	~CShInstruction();
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
private:
	int64_t RamPos;
	
	int getImmediateBits(ShImmediateType type);

	// opcode variables
	ShOpcodeData opcodeData;
	ShImmediateData immediateData;
	ShRegisterData registerData;
};
