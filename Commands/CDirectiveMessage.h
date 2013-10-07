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
	bool Load(CArgumentList& Args, int type);
	virtual bool Validate();
	virtual void Encode() {};
	virtual void WriteTempData(FILE*& Output) {};
private:
	char Message[1024];
	int ErrorType;
};

class CDirectiveSym: public CAssemblerCommand
{
public:
	CDirectiveSym(bool enable) {enabeld = enable; };
	virtual bool Validate() { return false; };
	virtual void Encode() { if(Global.SymData.Handle != NULL) Global.SymData.Write = enabeld; };
	virtual void WriteTempData(FILE*& Output) { };
private:
	bool enabeld;
};
