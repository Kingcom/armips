#pragma 
#include "Util/CommonClasses.h"
#include "Util/FileClasses.h"

void parseMacroDefinition(TextFile& Input, std::wstring& Args);
void LoadAssemblyFile(const std::wstring& fileName, TextFile::Encoding encoding = TextFile::GUESS);
bool EncodeAssembly();
