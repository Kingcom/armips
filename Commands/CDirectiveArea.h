#pragma once
#include "Commands/CAssemblerCommand.h"
#include "Core/Expression.h"

class CDirectiveArea: public CAssemblerCommand
{
public:
	CDirectiveArea(Expression& size);
	~CDirectiveArea();
	virtual bool Validate();
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
	virtual void writeSymData(SymbolData& symData) const;
	void setFillExpression(Expression& exp);
	void setContent(CAssemblerCommand* content) { this->content = content; }
private:
	u64 position;
	Expression sizeExpression;
	size_t areaSize;
	size_t contentSize;
	Expression fillExpression;
	u8 fillValue;
	CAssemblerCommand* content;
};
