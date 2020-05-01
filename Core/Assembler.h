#pragma once
#include "../Util/FileClasses.h"
#include "../Util/Util.h"
#include "FileManager.h"

#define ARMIPS_VERSION_MAJOR    0
#define ARMIPS_VERSION_MINOR    10
#define ARMIPS_VERSION_REVISION 0

enum class ArmipsMode { FILE, MEMORY };

struct LabelDefinition
{
	std::wstring originalName;
	std::wstring name;
	int64_t value;
};

struct EquationDefinition
{
	std::wstring name;
	std::wstring value;
};

struct ArmipsArguments
{
	// common
	ArmipsMode mode;
	int symFileVersion;
	bool errorOnWarning;
	bool silent;
	StringList* errorsResult;
	std::vector<EquationDefinition> equList;
	std::vector<LabelDefinition> labels;

	// file mode
	std::wstring inputFileName;
	std::wstring tempFileName;
	std::wstring symFileName;
	bool useAbsoluteFileNames;

	// memory mode
	std::shared_ptr<AssemblerFile> memoryFile;
	std::wstring content;

	ArmipsArguments()
	{
		mode = ArmipsMode::FILE;
		symFileVersion = 0;
		errorOnWarning = false;
		silent = false;
		errorsResult = nullptr;
		useAbsoluteFileNames = true;
	}
};

bool runArmips(ArmipsArguments& settings);
