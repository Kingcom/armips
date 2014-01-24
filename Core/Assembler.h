#pragma 
#include "Util/CommonClasses.h"
#include "Util/FileClasses.h"

struct AssemblerArguments
{
	std::wstring inputFileName;
	std::wstring tempFileName;
	std::wstring symFileName;
	int symFileVersion;
	std::wstring exSymFileName;
	bool errorOnWarning;
};

bool runAssembler(AssemblerArguments& arguments);
void parseMacroDefinition(TextFile& Input, std::wstring& Args);
void LoadAssemblyFile(const std::wstring& fileName, TextFile::Encoding encoding = TextFile::GUESS);
bool EncodeAssembly();
