#include "stdafx.h"
#include "Commands/CAssemblerCommand.h"
#include "Core/Common.h"

CAssemblerCommand::CAssemblerCommand()
{
	FileNum = Global.FileInfo.FileNum;
	FileLine = Global.FileInfo.LineNumber;
}

void CAssemblerCommand::SetFileInfo()
{
	Global.FileInfo.FileNum = FileNum;
	Global.FileInfo.LineNumber = FileLine;
}