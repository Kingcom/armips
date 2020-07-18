#include "Core/Common.h"

#include "Commands/CAssemblerLabel.h"
#include "Core/Assembler.h"
#include "Core/FileManager.h"
#include "Util/Util.h"

#include <sys/stat.h>

FileManager fileManager;
FileManager* g_fileManager = &fileManager;

tGlobal Global;
CArchitecture* Arch;

fs::path getFullPathName(const fs::path& path)
{
	if (Global.relativeInclude && !path.is_absolute())
	{
		const fs::path &source = Global.FileInfo.FileList[Global.FileInfo.FileNum];
		return fs::absolute(source.parent_path() / path).lexically_normal();
	}
	else
	{
		return fs::absolute(path).lexically_normal();
	}
}

bool checkLabelDefined(const std::wstring& labelName, int section)
{
	std::shared_ptr<Label> label = Global.symbolTable.getLabel(labelName,Global.FileInfo.FileNum,section);
	return label->isDefined();
}

bool checkValidLabelName(const std::wstring& labelName)
{
	return Global.symbolTable.isValidSymbolName(labelName);
}

bool isPowerOfTwo(int64_t n)
{
	if (n == 0) return false;
	return !(n & (n - 1));
}
