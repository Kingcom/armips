#pragma once
#include "Util/CommonClasses.h"
#include "Commands/CAssemblerCommand.h"
#include "Core/Common.h"

#define USERMESSAGE_WARNING		0
#define USERMESSAGE_ERROR		1
#define USERMESSAGE_NOTICE		2

class CDirectiveMessage: public CAssemblerCommand
{
public:
	CDirectiveMessage() {ErrorType = -1; };
	bool Load(ArgumentList& Args, int type);
	virtual bool Validate();
	virtual void Encode() {};
	virtual void writeTempData(TempData& tempData) { };
private:
	std::wstring message;
	int ErrorType;
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
