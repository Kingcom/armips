#include "stdafx.h"
#include "Commands/CDirectiveMessage.h"
#include "Core/Common.h"

bool CDirectiveMessage::Load(CArgumentList &Args, int type)
{
	strcpy(Message,Args.GetEntry(0));
	ErrorType = type;
	return true;
}


bool CDirectiveMessage::Validate()
{
	switch (ErrorType)
	{
	case USERMESSAGE_WARNING:
		QueueError(ERROR_WARNING,"%s",Message);
		break;
	case USERMESSAGE_ERROR:
		QueueError(ERROR_ERROR,"%s",Message);
		break;
	case USERMESSAGE_NOTICE:
		QueueError(ERROR_NOTICE,"%s",Message);
		break;
	}
	return false;
}