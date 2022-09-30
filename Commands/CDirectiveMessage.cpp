#include "Commands/CDirectiveMessage.h"

#include "Core/Common.h"
#include "Core/Misc.h"
#include "Core/SymbolData.h"

CDirectiveMessage::CDirectiveMessage(Type type, Expression exp)
	: errorType(type), exp(exp)
{
}

bool CDirectiveMessage::Validate(const ValidateState &state)
{
	StringLiteral text;
	if (!exp.evaluateString(text,true))
	{
		Logger::queueError(Logger::Error, "Invalid expression");
		return false;
	}

	switch (errorType)
	{
	case Type::Warning:
		Logger::queueError(Logger::Warning,text.string());
		break;
	case Type::Error:
		Logger::queueError(Logger::Error,text.string());
		break;
	case Type::Notice:
		Logger::queueError(Logger::Notice,text.string());
		break;
	}
	return false;
}

void CDirectiveSym::writeSymData(SymbolData &symData) const
{
	symData.setEnabled(enabled);
}
