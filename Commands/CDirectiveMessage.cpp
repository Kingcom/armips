#include "stdafx.h"
#include "Commands/CDirectiveMessage.h"
#include "Core/Common.h"

bool CDirectiveMessage::Load(ArgumentList &Args, int type)
{
	message = Args[0].text;
	ErrorType = type;
	return true;
}


bool CDirectiveMessage::Validate()
{
	switch (ErrorType)
	{
	case USERMESSAGE_WARNING:
		Logger::queueError(Logger::Warning,L"%s",message.c_str());
		break;
	case USERMESSAGE_ERROR:
		Logger::queueError(Logger::Error,L"%s",message.c_str());
		break;
	case USERMESSAGE_NOTICE:
		Logger::queueError(Logger::Notice,L"%s",message.c_str());
		break;
	}
	return false;
}