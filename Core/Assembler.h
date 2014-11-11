#pragma 
#include "Util/CommonClasses.h"
#include "Util/FileClasses.h"
#include "Util/Util.h"

struct AssemblerArguments
{
	std::wstring inputFileName;
	std::wstring tempFileName;
	std::wstring symFileName;
	int symFileVersion;
	bool errorOnWarning;
	bool silent;
	StringList* errorsResult;
	StringList equList;

	AssemblerArguments()
	{
		errorOnWarning = false;
		silent = false;
		errorsResult = NULL;
	}
};

bool runAssembler(AssemblerArguments& arguments);
void parseMacroDefinition(TextFile& Input, std::wstring& Args);
void LoadAssemblyFile(const std::wstring& fileName, TextFile::Encoding encoding = TextFile::GUESS);
bool EncodeAssembly();
