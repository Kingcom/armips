#pragma 
#include "Util/CommonClasses.h"
#include "Util/FileClasses.h"

bool ConditionalAssemblyTrue();
void parseMacroDefinition(TextFile& Input, std::wstring& Args);
void LoadAssemblyFile(std::wstring& fileName);
bool EncodeAssembly();
