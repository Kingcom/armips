#pragma once
#include "Util/CommonClasses.h"
#include "Commands/CAssemblerCommand.h"
#include "Core/MathParser.h"


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
	CExpressionCommandList SizeExpression;
	CExpressionCommandList ByteExpression;
	size_t Size;
	bool FillByte;
	int Byte;
	u64 RamPos;
};
