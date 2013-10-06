#pragma once
#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include "CLabels.h"
#include "Commands/CAssemblerCommand.h"
#include "Util/CommonClasses.h"
#include "CMacro.h"
#include "Util/EncodingTable.h"
#include "Util/FileClasses.h"

typedef struct {
	CStringList FileList;
	int FileCount;
	int FileNum;
	int LineNumber;
	int TotalLineCount;
} tFileInfo;

typedef struct {
	char Name[256];
	bool Write;
	FILE* Handle;
} tOutputFileData;

typedef struct {
	bool ConditionMet;
	bool ConditionTrue;
	bool ElseCase;
} tConditionEntry;

typedef struct {
	int MaxRamPos;
	int FileNum;
	int LineNumber;
} tAreaEntry;

typedef struct {
	std::vector<CAssemblerCommand*> Commands;
	std::vector<CMacro*> Macros;
	tFileInfo FileInfo;
	tOutputFileData TempData;
	tOutputFileData SymData;
	CLabels Labels;
	CErrorQueue ErrorQueue;
	EncodingTable Table;
//	HANDLE hFile;
	BinaryFile Output;
	int RamPos;
	int HeaderSize;
	bool FileOpened;
	int Section;
	int Radix;
	bool Error;
	bool Quit;
	bool Revalidate;
	bool nocash;
	bool relativeInclude;
	bool warningAsError;
	int DebugMessages;
	struct {
		tConditionEntry Entries[128];
		int EntryCount;
	} ConditionData; 
	struct {
		tAreaEntry Entries[128];
		int EntryCount;
	} AreaData;
	int IncludeNestingLevel;
	int MacroNestingLevel;
	CStringList Arguments;
	int validationPasses;
} tGlobal;

typedef enum eErrorLevel {ERROR_WARNING, ERROR_ERROR, ERROR_FATALERROR, ERROR_NOTICE };

extern tGlobal Global;
extern CArchitecture* Arch;

void getFolderNameFromPath(char* src, char* dest);
bool CheckLabelDefined(char* LabelName);
bool CheckValidLabelName(char* LabelName);
bool CheckLabelExists(char* LabelName);
bool AddAssemblerLabel(char* LabelName);
int FileSize( const char * szFileName );
bool FileExists(char* strFilename);
void AddAssemblerCommand(CAssemblerCommand* Command);
void QueueError(eErrorLevel level, char* format, ...);
void PrintError(eErrorLevel level, char* format, ...);
int ReadFileToBuffer(char* FileName, unsigned char** Pointer);
void WriteToTempData(FILE*& Output, char* str, int RamPos);
void WriteTempFile();
bool isPowerOfTwo(int n);
bool CheckBom(FILE*& Handle);
void getFullPathName(char* dest, char* path);
#endif