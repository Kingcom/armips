#pragma once

#include "Commands/CAssemblerCommand.h"
#include "Core/Common.h"
#include "Core/Expression.h"

class CDirectiveMessage: public CAssemblerCommand
{
public:
	enum class Type { Warning, Error, Notice };
	CDirectiveMessage(Type type, Expression exp);
	bool Validate(const ValidateState &state) override;
	virtual void Encode() const {};
	virtual void writeTempData(TempData& tempData) const { };
private:
	Type errorType;
	Expression exp;
};

class CDirectiveSym: public CAssemblerCommand
{
public:
	CDirectiveSym(bool enable) {enabled = enable; };
	bool Validate(const ValidateState &state) override { return false; }
	virtual void Encode() const { };
	virtual void writeTempData(TempData& tempData) const { };
	virtual void writeSymData(SymbolData& symData) const;
private:
	bool enabled;
};
