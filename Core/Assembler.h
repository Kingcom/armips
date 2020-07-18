#pragma once

#include "Util/FileSystem.h"

#include <memory>
#include <string>
#include <vector>

class AssemblerFile;

#define ARMIPS_VERSION_MAJOR    0
#define ARMIPS_VERSION_MINOR    11
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
	bool showStats;
	std::vector<std::wstring>* errorsResult;
	std::vector<EquationDefinition> equList;
	std::vector<LabelDefinition> labels;

	// file mode
	fs::path inputFileName;
	fs::path tempFileName;
	fs::path symFileName;
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
		showStats = false;
		errorsResult = nullptr;
		useAbsoluteFileNames = true;
	}
};

bool runArmips(ArmipsArguments& settings);
