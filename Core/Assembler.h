#pragma once
#include "../Util/FileClasses.h"
#include "../Util/Util.h"
#include "FileManager.h"

enum class ArmipsMode { FILE, MEMORY };

struct LabelDefinition
{
	std::wstring name;
	int value;
};

struct ArmipsArguments
{
	// common
	ArmipsMode mode;
	int symFileVersion;
	bool errorOnWarning;
	bool silent;
	StringList* errorsResult;
	StringList equList;
	std::vector<LabelDefinition> labels;

	// file mode
	std::wstring inputFileName;
	std::wstring tempFileName;
	std::wstring symFileName;

	// memory mode
	AssemblerFile* memoryFile;
	std::wstring content;

	ArmipsArguments()
	{
		mode = ArmipsMode::FILE;
		errorOnWarning = false;
		silent = false;
		errorsResult = NULL;
	}
};

bool runArmips(ArmipsArguments& arguments);
