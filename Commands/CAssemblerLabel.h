#pragma once

#include "Commands/CAssemblerCommand.h"
#include "Core/Expression.h"

class Label;

class CAssemblerLabel: public CAssemblerCommand
{
public:
	CAssemblerLabel(const std::wstring& name, const std::wstring& originalName);
	CAssemblerLabel(const std::wstring& name, const std::wstring& originalName, Expression& value);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
private:
	Expression labelValue;
	std::shared_ptr<Label> label;
	bool defined;
};

class CDirectiveFunction: public CAssemblerCommand
{
public:
	CDirectiveFunction(const std::wstring& name, const std::wstring& originalName);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
	void setContent(std::unique_ptr<CAssemblerCommand> content) { this->content = std::move(content); }
private:
	std::unique_ptr<CAssemblerLabel> label;
	std::unique_ptr<CAssemblerCommand> content;
	int64_t start, end;
};
