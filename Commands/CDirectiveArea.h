#pragma once

#include "Commands/CAssemblerCommand.h"
#include "Core/Expression.h"

class CDirectiveArea: public CAssemblerCommand
{
public:
	CDirectiveArea(bool shared, Expression& size);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
	void setFillExpression(Expression& exp);
	void setPositionExpression(Expression& exp);
	void setContent(std::unique_ptr<CAssemblerCommand> content) { this->content = std::move(content); }
private:
	bool shared;
	int64_t position = 0;
	Expression sizeExpression;
	int64_t areaSize = 0;
	int64_t contentSize = 0;
	Expression fillExpression;
	int8_t fillValue = 0;
	int64_t fileID = 0;
	Expression positionExpression;
	std::unique_ptr<CAssemblerCommand> content;
};

class CDirectiveAutoRegion : public CAssemblerCommand
{
public:
	CDirectiveAutoRegion();
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
	void setMinRangeExpression(Expression& exp);
	void setRangeExpressions(Expression& minExp, Expression& maxExp);
	void setContent(std::unique_ptr<CAssemblerCommand> content) { this->content = std::move(content); }

private:
	int64_t resetPosition;
	int64_t position;
	int64_t contentSize;
	int64_t fileID = 0;
	Expression minRangeExpression;
	Expression maxRangeExpression;
	std::unique_ptr<CAssemblerCommand> content;
};
