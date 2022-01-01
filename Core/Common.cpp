#include "Core/Common.h"

#include "Commands/CAssemblerLabel.h"
#include "Core/Assembler.h"
#include "Core/FileManager.h"
#include "Util/Util.h"

#include <sys/stat.h>

FileManager fileManager;
FileManager* g_fileManager = &fileManager;

tGlobal Global;

void FileList::add(const fs::path &path)
{
	_entries.emplace_back(path);
}

const fs::path &FileList::path(int fileIndex) const
{
	return _entries[size_t(fileIndex)].path();
}

const fs::path &FileList::relative_path(int fileIndex) const
{
	return _entries[size_t(fileIndex)].relativePath();
}

const std::string &FileList::string(int fileIndex) const
{
	return _entries[size_t(fileIndex)].string();
}

const std::string &FileList::relativeString(int fileIndex) const
{
	return _entries[size_t(fileIndex)].relativeString();
}

size_t FileList::size() const
{
	return _entries.size();
}

void FileList::clear()
{
	_entries.clear();
}

FileList::Entry::Entry(const fs::path &path) :
	_path(path),
	_relativePath(path.lexically_proximate(fs::current_path())),
	_string(_path.u8string()),
	_relativeString(_relativePath.generic_u8string())
{
}

const fs::path &FileList::Entry::path() const
{
	return _path;
}

const fs::path &FileList::Entry::relativePath() const
{
	return _relativePath;
}

const std::string &FileList::Entry::string() const
{
	return _string;
}

const std::string &FileList::Entry::relativeString() const
{
	return _relativeString;
}

fs::path getFullPathName(const fs::path& path)
{
	if (Global.relativeInclude && !path.is_absolute())
	{
		const fs::path &source = Global.fileList.path(Global.FileInfo.FileNum);
		return fs::absolute(source.parent_path() / path).lexically_normal();
	}
	else
	{
		return fs::absolute(path).lexically_normal();
	}
}

bool checkLabelDefined(const Identifier& labelName, int section)
{
	std::shared_ptr<Label> label = Global.symbolTable.getLabel(labelName,Global.FileInfo.FileNum,section);
	return label->isDefined();
}

bool checkValidLabelName(const Identifier& labelName)
{
	return Global.symbolTable.isValidSymbolName(labelName);
}

bool isPowerOfTwo(int64_t n)
{
	if (n == 0) return false;
	return !(n & (n - 1));
}
