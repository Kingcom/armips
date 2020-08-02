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
	void Encode() const override {};
	void writeTempData(TempData& tempData) const override { };
private:
	Type errorType;
	Expression exp;
};

class CDirectiveSym: public CAssemblerCommand
{
public:
	CDirectiveSym(bool enable) {enabled = enable; };
	bool Validate(const ValidateState &state) override { return false; }
	void Encode() const override { };
	void writeTempData(TempData& tempData) const override { };
	void writeSymData(SymbolData& symData) const override;
private:
	bool enabled;
};
