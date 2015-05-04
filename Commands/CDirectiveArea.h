#pragma once
#include "Util/CommonClasses.h"
#include "Commands/CAssemblerCommand.h"
#include "Core/Expression.h"

class CDirectiveArea: public CAssemblerCommand
{
public:
	CDirectiveArea(CAssemblerCommand* content, Expression& size);
	~CDirectiveArea();
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
	virtual void writeSymData(SymbolData& symData);
	void setFillExpression(Expression& exp);
private:
	u64 position;
	Expression sizeExpression;
	size_t areaSize;
	size_t contentSize;
	Expression fillExpression;
	u8 fillValue;
	CAssemblerCommand* content;
};
