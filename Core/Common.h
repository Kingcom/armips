#pragma once

#include <vector>
#include "SymbolTable.h"
#include "Commands/CAssemblerCommand.h"
#include "Util/CommonClasses.h"
#include "CMacro.h"
#include "Util/EncodingTable.h"
#include "Util/FileClasses.h"
#include "Core/Misc.h"

typedef struct {
	CStringList FileList;
	int FileCount;
	int FileNum;
	int LineNumber;
	int TotalLineCount;
} tFileInfo;

typedef struct {
	std::vector<CAssemblerCommand*> Commands;
	std::vector<CMacro*> Macros;
	tFileInfo FileInfo;
	TempData tempData;
	SymbolData symData;
	SymbolTable symbolTable;
	EncodingTable Table;
	BinaryFile Output;
	int RamPos;
	int HeaderSize;
	bool FileOpened;
	int Section;
	int Radix;
	bool Quit;
	bool Revalidate;
	bool nocash;
	bool relativeInclude;
	bool warningAsError;
	int DebugMessages;
	ConditionData conditionData;
	AreaData areaData;
	int IncludeNestingLevel;
	int MacroNestingLevel;
	CStringList Arguments;
	int validationPasses;
} tGlobal;

typedef enum eErrorLevel {ERROR_WARNING, ERROR_ERROR, ERROR_FATALERROR, ERROR_NOTICE };

extern tGlobal Global;
extern CArchitecture* Arch;

std::wstring getFolderNameFromPath(const std::wstring& src);
std::wstring getFullPathName(const std::wstring& path);

bool checkLabelDefined(const std::wstring& labelName);
bool checkValidLabelName(const std::wstring& labelName);
bool addAssemblerLabel(const std::wstring& labelName);

void AddAssemblerCommand(CAssemblerCommand* Command);
bool isPowerOfTwo(int n);
