#include "stdafx.h"
#include "Core/Common.h"
#include <sys/stat.h>
#include "Assembler.h"
#include "Commands/CAssemblerLabel.h"
#include "Util/Util.h"
#include "Core/FileManager.h"

FileManager fileManager;
FileManager* g_fileManager = &fileManager;

tGlobal Global;
CArchitecture* Arch;

std::wstring getFolderNameFromPath(const std::wstring& src)
{
	size_t s = src.rfind('\\');
	if (s == std::wstring::npos)
	{
		s = src.rfind('/');
		if (s == std::wstring::npos)
			return L".";
	}

	return src.substr(0,s);
}

std::wstring getFullPathName(const std::wstring& path)
{
	if (Global.relativeInclude == true)
	{
		if (path.size() >= 3 && path[1] == ':' && (path[2] == '/' || path[2] == '\\'))
		{
			return path;
		} else {
			std::wstring source = convertUtf8ToWString(Global.FileInfo.FileList.GetEntry(Global.FileInfo.FileNum));
			return getFolderNameFromPath(source) + L"/" + path;
		}
	} else {
		return path;
	}
}

bool checkLabelDefined(const std::wstring& labelName)
{
	Label* label = Global.symbolTable.getLabel(labelName,Global.FileInfo.FileNum,Global.Section);
	return label->isDefined();
}

bool checkValidLabelName(const std::wstring& labelName)
{
	return Global.symbolTable.isValidSymbolName(labelName);
}

bool addAssemblerLabel(const std::wstring& labelName)
{
	if (checkValidLabelName(labelName) == false)
	{
		Logger::printError(Logger::Error,L"Invalid label name \"%s\"",labelName);
		return false;
	}

	CAssemblerLabel* Label = new CAssemblerLabel(labelName,g_fileManager->getVirtualAddress(),Global.Section,false);
	AddAssemblerCommand(Label);
	return true;
}

void AddAssemblerCommand(CAssemblerCommand* Command)
{
	Global.Commands.push_back(Command);
}

bool isPowerOfTwo(int n)
{
	if (n == 0) return false;
	return !(n & (n - 1));
}
