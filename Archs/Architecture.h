#pragma once
#include "../Commands/CAssemblerCommand.h"
#include "../Core/ExpressionFunctions.h"
#include "../Core/FileManager.h"
#include "../Core/ELF/ElfRelocator.h"

class Tokenizer;
class Parser;

class CArchitecture
{
public:
	virtual std::unique_ptr<CAssemblerCommand> parseDirective(Parser& parser) { return nullptr; }
	virtual std::unique_ptr<CAssemblerCommand> parseOpcode(Parser& parser) { return nullptr; }
	virtual const ExpressionFunctionMap& getExpressionFunctions() { return emptyMap; }
	virtual void NextSection() = 0;
	virtual void Pass2() = 0;
	virtual void Revalidate() = 0;
	virtual std::unique_ptr<IElfRelocator> getElfRelocator() = 0;
	virtual Endianness getEndianness() = 0;
private:
	const ExpressionFunctionMap emptyMap = {};
};

class ArchitectureCommand: public CAssemblerCommand
{
public:
	ArchitectureCommand(const std::wstring& tempText, const std::wstring& symText);
	virtual bool Validate();
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
	virtual void writeSymData(SymbolData& symData) const;
private:
	int64_t position;
	Endianness endianness;
	std::wstring tempText;
	std::wstring symText;
};

class CInvalidArchitecture: public CArchitecture
{
public:
	virtual void NextSection();
	virtual void Pass2();
	virtual void Revalidate();
	virtual std::unique_ptr<IElfRelocator> getElfRelocator();
	virtual Endianness getEndianness() { return Endianness::Little; }
};

extern CInvalidArchitecture InvalidArchitecture;
