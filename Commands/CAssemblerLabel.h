#pragma once
#include "Commands/CAssemblerCommand.h"
#include "Core/Expression.h"

class Label;

class CAssemblerLabel: public CAssemblerCommand
{
public:
	CAssemblerLabel(const std::wstring& name, const std::wstring& originalName);
	CAssemblerLabel(const std::wstring& name, const std::wstring& originalName, Expression& value);
	virtual bool Validate();
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
	virtual void writeSymData(SymbolData& symData) const;
private:
	Expression labelValue;
	std::shared_ptr<Label> label;
	bool defined;
};

class CDirectiveFunction: public CAssemblerCommand
{
public:
	CDirectiveFunction(const std::wstring& name, const std::wstring& originalName);
	virtual bool Validate();
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
	virtual void writeSymData(SymbolData& symData) const;
	void setContent(std::unique_ptr<CAssemblerCommand> content) { this->content = std::move(content); }
private:
	std::unique_ptr<CAssemblerLabel> label;
	std::unique_ptr<CAssemblerCommand> content;
	int64_t start, end;
};
