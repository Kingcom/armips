#include "stdafx.h"
#include "Commands/CAssemblerCommand.h"
#include "Core/Common.h"

CAssemblerCommand::CAssemblerCommand()
{
	FileNum = Global.FileInfo.FileNum;
	FileLine = Global.FileInfo.LineNumber;
	section = Global.Section;
}

void CAssemblerCommand::SetFileInfo()
{
	Global.FileInfo.FileNum = FileNum;
	Global.FileInfo.LineNumber = FileLine;
}

CommentCommand::CommentCommand(const std::wstring& tempText, const std::wstring& symText)
{
	this->tempText = tempText;
	this->symText = symText;
}

bool CommentCommand::Validate()
{
	position = g_fileManager->getVirtualAddress();
	return false;
}

void CommentCommand::writeTempData(TempData& tempData) const
{
	if (tempText.size() != 0)
	{
		std::wstringstream stream(tempText);

		std::wstring line;
		while (std::getline(stream,line,L'\n'))
		{
			if (line.size() != 0)
				tempData.writeLine(position,line);
		}
	}
}

void CommentCommand::writeSymData(SymbolData& symData) const
{
	if (symText.size() != 0)
		symData.addLabel(position,symText);
}
