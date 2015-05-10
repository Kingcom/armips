#pragma once
#include "Commands/CAssemblerCommand.h"
#include "Core/Expression.h"

class Label;

class CAssemblerLabel: public CAssemblerCommand
{
public:
	CAssemblerLabel(const std::wstring& name);
	CAssemblerLabel(const std::wstring& name, Expression& value);
	virtual bool Validate();
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
	virtual void writeSymData(SymbolData& symData) const;
private:
	Expression labelValue;
	Label* label;
	bool defined;
};

class CDirectiveFunction: public CAssemblerCommand
{
public:
	CDirectiveFunction(const std::wstring& name, CAssemblerCommand* content);
	virtual ~CDirectiveFunction();
	virtual bool Validate();
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
	virtual void writeSymData(SymbolData& symData) const;
private:
	CAssemblerLabel* label;
	CAssemblerCommand* content;
};
