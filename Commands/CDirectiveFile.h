#pragma once
#include "Commands/CAssemblerCommand.h"
#include "Util/CommonClasses.h"
#include "Core/MathParser.h"
#include "Core/ELF/ElfRelocator.h"

class GenericAssemblerFile;

class CDirectiveFile: public CAssemblerCommand
{
public:
	enum class Type { Open, Create, Copy, Close };

	CDirectiveFile(Type type, ArgumentList& args);
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
	virtual void writeSymData(SymbolData& symData) { };
private:
	Type type;
	GenericAssemblerFile* file;
};

class CDirectivePosition: public CAssemblerCommand
{
public:
	enum Type { Physical, Virtual };
	CDirectivePosition(Type type, ArgumentList& args);
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
	virtual void writeSymData(SymbolData& symData) { };
private:
	void exec();
	Type type;
	int position;
};

class CDirectiveIncbin: public CAssemblerCommand
{
public:
	CDirectiveIncbin(ArgumentList& args);
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
	virtual void writeSymData(SymbolData& symData);
private:
	std::wstring fileName;
	int startAddress;
	int loadSize;
};

class CDirectiveAlign: public CAssemblerCommand
{
public:
	CDirectiveAlign(ArgumentList& args);
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
	virtual void writeSymData(SymbolData& symData) { };
private:
	int computePadding();
	int alignment;
	CExpressionCommandList fillExpression;
	int fillByte;
};

class CDirectiveHeaderSize: public CAssemblerCommand
{
public:
	CDirectiveHeaderSize(ArgumentList& args);
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData);
	virtual void writeSymData(SymbolData& symData) { };
private:
	void updateFile();
	int headerSize;
};

class DirectiveObjImport: public CAssemblerCommand
{
public:
	DirectiveObjImport(ArgumentList& args);
	~DirectiveObjImport() { };
	virtual bool Validate();
	virtual void Encode();
	virtual void writeTempData(TempData& tempData) { };
	virtual void writeSymData(SymbolData& symData);
private:
	ElfRelocator rel;
};