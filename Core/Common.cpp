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
			std::wstring source = Global.FileInfo.FileList[Global.FileInfo.FileNum];
			return getFolderNameFromPath(source) + L"/" + path;
		}
	} else {
		return path;
	}
}

bool checkLabelDefined(const std::wstring& labelName, int section)
{
	Label* label = Global.symbolTable.getLabel(labelName,Global.FileInfo.FileNum,section);
	return label->isDefined();
}

bool checkValidLabelName(const std::wstring& labelName)
{
	return Global.symbolTable.isValidSymbolName(labelName);
}

bool isPowerOfTwo(u64 n)
{
	if (n == 0) return false;
	return !(n & (n - 1));
}
