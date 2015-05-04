#pragma once
#include "Util/CommonClasses.h"
#include "Commands/CAssemblerCommand.h"
#include "Core/Common.h"
#include "Core/Expression.h"

class CDirectiveMessage: public CAssemblerCommand
{
public:
	enum class Type { Invalid, Warning, Error, Notice };	
	CDirectiveMessage(Type type, Expression exp);
	virtual bool Validate();
	virtual void Encode() {};
	virtual void writeTempData(TempData& tempData) { };
private:
	Type errorType;
	Expression exp;
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
