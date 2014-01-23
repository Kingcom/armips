#pragma once
#include "Util/CommonClasses.h"
#include "Commands/CAssemblerCommand.h"
#include "Core/Common.h"

class CDirectiveMessage: public CAssemblerCommand
{
public:
	enum class Type { Invalid, Warning, Error, Notice };
	CDirectiveMessage() { errorType = Type::Invalid; };
	bool Load(ArgumentList& Args, Type type);
	virtual bool Validate();
	virtual void Encode() {};
	virtual void writeTempData(TempData& tempData) { };
private:
	std::wstring message;
	Type errorType;
};

class CDirectiveSym: public CAssemblerCommand
{
public:
	CDirectiveSym(bool enable) {enabled = enable; };
	virtual bool Validate() { return false; };
	virtual void Encode() { };
	virtual void writeTempData(TempData& tempData) { };
	virtual void writeSymData(SymbolData& symData) { symData.setEnabled(enabled); }
private:
	bool enabled;
};
