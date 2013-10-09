#include "stdafx.h"
#include "Core/Common.h"
#include <sys/stat.h>
#include "Assembler.h"
#include "Commands/CAssemblerLabel.h"
#include "Util/Util.h"

tGlobal Global;
CArchitecture* Arch;
#include <direct.h>

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
		PrintError(ERROR_ERROR,"Invalid label name \"%ls\"",labelName.c_str());
		return false;
	}

	if (checkLabelDefined(labelName) == true)
	{
		PrintError(ERROR_ERROR,"Label \"%ls\" already defined",labelName.c_str());
		return false;
	}

	CAssemblerLabel* Label = new CAssemblerLabel(labelName,Global.RamPos,Global.Section,false);
	AddAssemblerCommand(Label);
	return true;
}

void AddAssemblerCommand(CAssemblerCommand* Command)
{
	Global.Commands.push_back(Command);
}

void QueueError(eErrorLevel level, char* format, ...)
{
	char str[1024];
	va_list args;

	va_start(args,format);
	vsprintf(str,format,args);
	va_end (args);

	Global.ErrorQueue.AddEntry(level,str);
}

void PrintError(eErrorLevel level, char* format, ...)
{
	char str[1024];
	va_list args;

	va_start(args,format);
	vsprintf(str,format,args);
	va_end (args);

	char* FileName = Global.FileInfo.FileList.GetEntry(Global.FileInfo.FileNum);

	switch (level)
	{
	case ERROR_WARNING:
		printf("%s(%d) warning: %s\n",FileName,Global.FileInfo.LineNumber,str);
		if (Global.warningAsError == true) Global.Error = true;
		break;
	case ERROR_ERROR:
		printf("%s(%d) error: %s\n",FileName,Global.FileInfo.LineNumber,str);
		Global.Error = true;
		break;
	case ERROR_FATALERROR:
		printf("%s(%d) fatal error: %s\n",FileName,Global.FileInfo.LineNumber,str);
		exit(2);
	case ERROR_NOTICE:
		printf("%s(%d) notice: %s\n",FileName,Global.FileInfo.LineNumber,str);
		break;
	}
}

bool isPowerOfTwo(int n)
{
	if (n == 0) return false;
	return !(n & (n - 1));
}
