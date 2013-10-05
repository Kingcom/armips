#pragma once
#include "Util/CommonClasses.h"
#include "Commands/CAssemblerCommand.h"
#include "Core/MathParser.h"


class CDirectiveFill: public CAssemblerCommand
{
public:
	CDirectiveFill();
	bool Load(CArgumentList& Args);
	virtual bool Validate();
	virtual void Encode();
	virtual void WriteTempData(FILE*& Output);
private:
	CExpressionCommandList SizeExpression;
	CExpressionCommandList ByteExpression;
	int Size;
	bool FillByte;
	int Byte;
	int RamPos;
};
