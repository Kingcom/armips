#pragma once

#include "Commands/CAssemblerCommand.h"
#include "Util/ByteArray.h"

#include <map>
#include <memory>

class ExpressionFunctionHandler;
class IElfRelocator;
class Tokenizer;
class Parser;

class Architecture
{
public:
	static Architecture &current();
	static void setCurrent(Architecture &arch);

	virtual std::unique_ptr<CAssemblerCommand> parseDirective(Parser& parser) { return nullptr; }
	virtual std::unique_ptr<CAssemblerCommand> parseOpcode(Parser& parser) { return nullptr; }
	virtual void registerExpressionFunctions(ExpressionFunctionHandler &handler);
	virtual void NextSection() = 0;
	virtual void Pass2() = 0;
	virtual void Revalidate() = 0;
	virtual std::unique_ptr<IElfRelocator> getElfRelocator() = 0;
	virtual Endianness getEndianness() = 0;
	virtual int getWordSize() = 0;
private:
	static Architecture *currentArchitecture;
};

class ArchitectureCommand: public CAssemblerCommand
{
public:
	ArchitectureCommand(const std::string& tempText, const std::string& symText);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
private:
	Architecture *architecture = nullptr;
	int64_t position;
	Endianness endianness;
	std::string tempText;
	std::string symText;
};

class CInvalidArchitecture: public Architecture
{
public:
	void NextSection() override;
	void Pass2() override;
	void Revalidate() override;
	std::unique_ptr<IElfRelocator> getElfRelocator() override;
	Endianness getEndianness() override { return Endianness::Little; }
	int getWordSize() override { return 4; };
};

extern CInvalidArchitecture InvalidArchitecture;
