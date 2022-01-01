#pragma once

#include "Commands/CAssemblerCommand.h"
#include "Core/Expression.h"
#include "Core/Types.h"

enum class ConditionType
{
	IF,
	ELSE,
	ELSEIF,
	ENDIF,
	IFDEF,
	IFNDEF,
};

class CDirectiveConditional: public CAssemblerCommand
{
public:
	CDirectiveConditional(ConditionType type);
	CDirectiveConditional(ConditionType type, const Identifier& name);
	CDirectiveConditional(ConditionType type, const Expression& exp);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
	void setContent(std::unique_ptr<CAssemblerCommand> ifBlock, std::unique_ptr<CAssemblerCommand> elseBlock);
private:
	bool evaluate();

	Expression expression;
	std::shared_ptr<Label> label;
	bool previousResult;

	ConditionType type;
	std::unique_ptr<CAssemblerCommand> ifBlock;
	std::unique_ptr<CAssemblerCommand> elseBlock;
};
