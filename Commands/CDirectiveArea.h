#pragma once
#include "Util/CommonClasses.h"
#include "Commands/CAssemblerCommand.h"
#include "Core/MathParser.h"

class CDirectiveArea: public CAssemblerCommand
{
public:
	CDirectiveArea();
	bool LoadStart(ArgumentList& Args);
	bool LoadEnd();
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
private:
	bool Start;
	CExpressionCommandList SizeExpression;
	int Size;
	int RamPos;
	int Fill;
	bool UseFill;
	CExpressionCommandList FillExpression;
};

