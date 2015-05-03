#pragma once
#include "Util/CommonClasses.h"
#include "Commands/CAssemblerCommand.h"
#include "Core/Expression.h"

enum class ConditionType
{
	IF,
	ELSE,
	ELSEIF,
	ENDIF,
	IFDEF,
	IFNDEF,
	ELSEIFDEF,
	ELSEIFNDEF,
	IFARM,
	IFTHUMB
};

class CDirectiveConditional: public CAssemblerCommand
{
public:
	CDirectiveConditional(ConditionType type);
	CDirectiveConditional(ConditionType type, const std::wstring& name);
	CDirectiveConditional(ConditionType type, const Expression& exp);
	~CDirectiveConditional();
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData) { };
	void setContent(CAssemblerCommand* ifBlock, CAssemblerCommand* elseBlock);
private:
	bool evaluate();

	Expression expression;
	Label* label;
	u32 armState;
	bool previousResult;

	ConditionType type;
	CAssemblerCommand* ifBlock;
	CAssemblerCommand* elseBlock;
};
