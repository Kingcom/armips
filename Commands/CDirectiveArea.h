#pragma once
#include "Util/CommonClasses.h"
#include "Commands/CAssemblerCommand.h"
#include "Core/MathParser.h"

class CDirectiveArea: public CAssemblerCommand
{
public:
	CDirectiveArea();
	bool LoadStart(CArgumentList& Args);
	bool LoadEnd();
	virtual bool Validate();
	virtual void Encode();
	virtual void WriteTempData(FILE*& Output);
private:
	bool Start;
	CExpressionCommandList SizeExpression;
	int Size;
	int RamPos;
	int Fill;
	bool UseFill;
	CExpressionCommandList FillExpression;
};

