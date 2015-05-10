#pragma once
#include "Commands/CAssemblerCommand.h"
#include "Core/Expression.h"
#include "Core/ELF/ElfRelocator.h"

class GenericAssemblerFile;

class CDirectiveFile: public CAssemblerCommand
{
public:
	enum class Type { Invalid, Open, Create, Copy, Close };

	CDirectiveFile();
	void initOpen(const std::wstring& fileName, u64 memory);
	void initCreate(const std::wstring& fileName, u64 memory);
	void initCopy(const std::wstring& inputName, const std::wstring& outputName, u64 memory);
	void initClose();

	virtual bool Validate();
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
	virtual void writeSymData(SymbolData& symData) const { };
private:
	Type type;
	GenericAssemblerFile* file;
};

class CDirectivePosition: public CAssemblerCommand
{
public:
	enum Type { Physical, Virtual };
	CDirectivePosition(Type type, u64 position);
	virtual bool Validate();
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
	virtual void writeSymData(SymbolData& symData) const { };
private:
	void exec() const;
	Type type;
	u64 position;
};

class CDirectiveIncbin: public CAssemblerCommand
{
public:
	CDirectiveIncbin(const std::wstring& fileName);
	void setStart(Expression& exp) { startExpression = exp; };
	void setSize(Expression& exp) { sizeExpression = exp; };

	virtual bool Validate();
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
	virtual void writeSymData(SymbolData& symData) const;
private:
	std::wstring fileName;
	size_t fileSize;

	Expression startExpression;
	Expression sizeExpression;
	u64 start;
	u64 size;
};

class CDirectiveAlignFill: public CAssemblerCommand
{
public:
	enum Mode { Align, Fill };

	CDirectiveAlignFill(u64 value, Mode mode);
	CDirectiveAlignFill(Expression& value, Mode mode);
	CDirectiveAlignFill(Expression& value, Expression& fillValue, Mode mode);
	virtual bool Validate();
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
	virtual void writeSymData(SymbolData& symData) const;
private:

	Mode mode;
	Expression valueExpression;
	Expression fillExpression;
	u64 value;
	u64 finalSize;
	u8 fillByte;
};

class CDirectiveHeaderSize: public CAssemblerCommand
{
public:
	CDirectiveHeaderSize(u64 size);
	virtual bool Validate();
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
	virtual void writeSymData(SymbolData& symData) const { };
private:
	void updateFile() const;
	u64 headerSize;
};

class DirectiveObjImport: public CAssemblerCommand
{
public:
	DirectiveObjImport(const std::wstring& inputName);
	DirectiveObjImport(const std::wstring& inputName, const std::wstring& ctorName);
	~DirectiveObjImport() { };
	virtual bool Validate();
	virtual void Encode() const;
	virtual void writeTempData(TempData& tempData) const;
	virtual void writeSymData(SymbolData& symData) const;
private:
	ElfRelocator rel;
	CAssemblerCommand* ctor;
};