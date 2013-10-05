#pragma once
#include "Util/CommonClasses.h"

bool ConditionalAssemblyTrue();
void ParseMacroDefinition(FILE*& Input, char* Args);
void LoadAssemblyFile(char* FileName);
bool EncodeAssembly();
