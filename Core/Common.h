#pragma once

#include "Core/SymbolTable.h"
#include "Util/EncodingTable.h"
#include "Util/FileSystem.h"

#include <string>
#include <vector>

class AssemblerFile;
class CArchitecture;

class FileList
{
public:
	void add(const fs::path& path);

	const fs::path& path(int fileIndex) const;
	const fs::path& relative_path(int fileIndex) const;
	const std::string& string(int fileIndex) const;
	const std::string& relativeString(int fileIndex) const;

	size_t size() const;
	void clear();

private:
	class Entry
	{
	public:
		Entry(const fs::path& path);

		const fs::path& path() const;
		const fs::path& relativePath() const;
		const std::string& string() const;
		const std::string& relativeString() const;

	private:
		fs::path _path;
		fs::path _relativePath;
		std::string _string; // preconverted for performance
		std::string _relativeString; // preconverted for performance
	};

	std::vector<Entry> _entries;
};

typedef struct {
	int FileNum;
	int LineNumber;
	int TotalLineCount;
} tFileInfo;

typedef struct {
	FileList fileList;
	tFileInfo FileInfo;
	SymbolTable symbolTable;
	EncodingTable Table;
	int Section;
	bool nocash;
	bool relativeInclude;
	bool memoryMode;
	std::shared_ptr<AssemblerFile> memoryFile;
	bool multiThreading;
} tGlobal;

extern tGlobal Global;

class FileManager;
extern FileManager* g_fileManager;

fs::path getFullPathName(const fs::path& path);

bool checkLabelDefined(const Identifier& labelName, int section);
bool checkValidLabelName(const Identifier& labelName);

bool isPowerOfTwo(int64_t n);
