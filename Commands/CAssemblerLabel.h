#pragma once

#include "Commands/CAssemblerCommand.h"
#include "Core/Expression.h"
#include "Core/Types.h"

#include <optional>

class Label;

class CAssemblerLabel: public CAssemblerCommand
{
public:
	CAssemblerLabel(const Identifier& name, const Identifier& originalName, std::optional<bool> thumbMode = std::nullopt);
	CAssemblerLabel(const Identifier& name, const Identifier& originalName, Expression& value, std::optional<bool> thumbMode = std::nullopt);
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
	CDirectiveFunction(const Identifier& name, const Identifier& originalName);
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
