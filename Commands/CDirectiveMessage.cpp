#include "stdafx.h"
#include "Commands/CDirectiveMessage.h"
#include "Core/Common.h"

bool CDirectiveMessage::Load(ArgumentList &Args, Type type)
{
	message = Args[0].text;
	errorType = type;
	return true;
}


bool CDirectiveMessage::Validate()
{
	switch (errorType)
	{
	case Type::Warning:
		Logger::queueError(Logger::Warning,L"%s",message.c_str());
		break;
	case Type::Error:
		Logger::queueError(Logger::Error,L"%s",message.c_str());
		break;
	case Type::Notice:
		Logger::queueError(Logger::Notice,L"%s",message.c_str());
		break;
	}
	return false;
}