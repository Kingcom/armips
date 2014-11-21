#pragma 
#include "../Util/CommonClasses.h"
#include "../Util/FileClasses.h"
#include "../Util/Util.h"
#include "FileManager.h"

enum class ArmipsMode { File, Memory };

struct ArmipsArguments
{
	// common
	ArmipsMode mode;
	int symFileVersion;
	bool errorOnWarning;
	bool silent;
	StringList* errorsResult;
	StringList equList;

	// file mode
	std::wstring inputFileName;
	std::wstring tempFileName;
	std::wstring symFileName;

	// memory mode
	AssemblerFile* memoryFile;
	std::wstring content;

	ArmipsArguments()
	{
		mode = ArmipsMode::File;
		errorOnWarning = false;
		silent = false;
		errorsResult = NULL;
	}
};

bool runArmips(ArmipsArguments& arguments);
void parseMacroDefinition(TextFile& Input, std::wstring& Args);
void LoadAssemblyFile(const std::wstring& fileName, TextFile::Encoding encoding = TextFile::GUESS);
bool EncodeAssembly();
