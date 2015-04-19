#pragma once
#include "Util/CommonClasses.h"
#include "Commands/CAssemblerCommand.h"
#include "Core/Expression.h"


class CDirectiveFill: public CAssemblerCommand
{
public:
	CDirectiveFill();
	bool Load(ArgumentList& Args);
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
	virtual void writeSymData(SymbolData& symData);
private:
	Expression sizeExpression;
	Expression byteExpression;
	size_t Size;
	bool FillByte;
	u8 Byte;
	u64 RamPos;
};
