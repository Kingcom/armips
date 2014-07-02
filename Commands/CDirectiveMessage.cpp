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
		Logger::queueError(Logger::Warning,message);
		break;
	case Type::Error:
		Logger::queueError(Logger::Error,message);
		break;
	case Type::Notice:
		Logger::queueError(Logger::Notice,message);
		break;
	}
	return false;
}