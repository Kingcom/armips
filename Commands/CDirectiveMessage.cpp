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
		QueueError(ERROR_WARNING,"%ls",message.c_str());
		break;
	case USERMESSAGE_ERROR:
		QueueError(ERROR_ERROR,"%ls",message.c_str());
		break;
	case USERMESSAGE_NOTICE:
		QueueError(ERROR_NOTICE,"%ls",message.c_str());
		break;
	}
	return false;
}