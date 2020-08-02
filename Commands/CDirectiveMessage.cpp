#include "Commands/CDirectiveMessage.h"

#include "Core/Common.h"
#include "Core/Misc.h"
#include "Core/SymbolData.h"

CDirectiveMessage::CDirectiveMessage(Type type, Expression exp)
{
	errorType = type;
	this->exp = exp;
}

bool CDirectiveMessage::Validate(const ValidateState &state)
{
	std::wstring text;
	if (!exp.evaluateString(text,true))
	{
		Logger::queueError(Logger::Error,L"Invalid expression");
		return false;
	}

	switch (errorType)
	{
	case Type::Warning:
		Logger::queueError(Logger::Warning,text);
		break;
	case Type::Error:
		Logger::queueError(Logger::Error,text);
		break;
	case Type::Notice:
		Logger::queueError(Logger::Notice,text);
		break;
	}
	return false;
}

void CDirectiveSym::writeSymData(SymbolData &symData) const
{
	symData.setEnabled(enabled);
}
