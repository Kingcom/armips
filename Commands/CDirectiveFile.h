#pragma once

#include "Commands/CAssemblerCommand.h"
#include "Core/ELF/ElfRelocator.h"
#include "Core/Expression.h"
#include "Util/FileSystem.h"

class AssemblerFile;
class GenericAssemblerFile;

class CDirectiveFile: public CAssemblerCommand
{
public:
	enum class Type { Invalid, Open, Create, Copy, Close };

	CDirectiveFile();
	void initOpen(const fs::path& fileName, int64_t memory);
	void initCreate(const fs::path& fileName, int64_t memory);
	void initCopy(const fs::path& inputName, const fs::path& outputName, int64_t memory);
	void initClose();

	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
private:
	Type type;
	int64_t virtualAddress;
	std::shared_ptr<GenericAssemblerFile> file;
	std::shared_ptr<AssemblerFile> closeFile;
};

class CDirectivePosition: public CAssemblerCommand
{
public:
	enum Type { Physical, Virtual };
	CDirectivePosition(Expression value, Type type);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override { };
private:
	void exec() const;
	Expression expression;
	Type type;
	int64_t position;
	int64_t virtualAddress;
};

class CDirectiveIncbin: public CAssemblerCommand
{
public:
	CDirectiveIncbin(const fs::path& fileName);
	void setStart(Expression& exp) { startExpression = exp; };
	void setSize(Expression& exp) { sizeExpression = exp; };

	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
private:
	fs::path fileName;
	int64_t fileSize;

	Expression startExpression;
	Expression sizeExpression;
	int64_t size;
	int64_t start;
	int64_t virtualAddress;
};

class CDirectiveAlignFill: public CAssemblerCommand
{
public:
	enum Mode { AlignPhysical, AlignVirtual, Fill };

	CDirectiveAlignFill(int64_t value, Mode mode);
	CDirectiveAlignFill(Expression& value, Mode mode);
	CDirectiveAlignFill(Expression& value, Expression& fillValue, Mode mode);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
private:
	Mode mode;
	Expression valueExpression;
	Expression fillExpression;
	int64_t value;
	int64_t finalSize;
	int8_t fillByte;
	int64_t virtualAddress;
};

class CDirectiveSkip: public CAssemblerCommand
{
public:
	CDirectiveSkip(Expression& value);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override { };
private:
	Expression expression;
	int64_t value;
	int64_t virtualAddress;
};

class CDirectiveHeaderSize: public CAssemblerCommand
{
public:
	CDirectiveHeaderSize(Expression expression);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override { };
private:
	void exec() const;
	Expression expression;
	int64_t headerSize;
	int64_t virtualAddress;
};

class DirectiveObjImport: public CAssemblerCommand
{
public:
	DirectiveObjImport(const fs::path& inputName);
	DirectiveObjImport(const fs::path& inputName, const Identifier& ctorName);
	~DirectiveObjImport() { };
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
	inline bool isSuccessfullyImported() { return success; };
private:
	bool success;
	ElfRelocator rel;
	std::unique_ptr<CAssemblerCommand> ctor;
};
