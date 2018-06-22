#pragma once
#include "Commands/CAssemblerCommand.h"
#include "Core/Expression.h"

class CDirectiveArea: public CAssemblerCommand
{
public:
	CDirectiveArea(Expression& size);
	virtual bool Validate();
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
	virtual void writeSymData(SymbolData& symData) const;
	void setFillExpression(Expression& exp);
	void setContent(std::unique_ptr<CAssemblerCommand> content) { this->content = std::move(content); }
private:
	int64_t position;
	Expression sizeExpression;
	int64_t areaSize;
	int64_t contentSize;
	Expression fillExpression;
	int8_t fillValue;
	std::unique_ptr<CAssemblerCommand> content;
};
