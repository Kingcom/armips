#include "stdafx.h"
#include "Core/Common.h"
#include <sys/stat.h>
#include "Assembler.h"
#include "Commands/CAssemblerLabel.h"
#include "Util/Util.h"

tGlobal Global;
CArchitecture* Arch;
#include <direct.h>

void getFolderNameFromPath(char* src, char* dest)
{
	char* s = strrchr(src,'\\');
	if (s == NULL)
	{
		s = strrchr(src,'/');
		if (s == NULL)
		{
			strcpy(dest,".");
			return;
		}
	}

	memcpy(dest,src,s-src);
	dest[s-src] = 0;
}

void getFullPathName(char* dest, char* path)
{
	if (Global.relativeInclude == true)
	{
		if (path[1] == ':' && (path[2] == '/' || path[2] == '\\'))
		{
			strcpy(dest,path);
		} else {
			char WorkingFolder[255];
			getFolderNameFromPath(Global.FileInfo.FileList.GetEntry(Global.FileInfo.FileNum),WorkingFolder);
			sprintf(dest,"%s\\%s",WorkingFolder,path);
		}
	} else {
		strcpy(dest,path);
	}
}

bool CheckLabelDefined(char* LabelName)
{
	Label* label = Global.symbolTable.getLabel(convertUtf8ToWString(LabelName),Global.FileInfo.FileNum,Global.Section);
	return label->isDefined();
}

bool CheckValidLabelName(char* LabelName)
{
	return Global.symbolTable.isValidSymbolName(convertUtf8ToWString(LabelName));
}

bool AddAssemblerLabel(char* LabelName)
{
	if (CheckValidLabelName(LabelName) == false)
	{
		PrintError(ERROR_ERROR,"Invalid label name \"%s\"",LabelName);
		return false;
	}

	if (CheckLabelDefined(LabelName) == true)
	{
		PrintError(ERROR_ERROR,"Label \"%s\" already defined",LabelName);
		return false;
	}

	std::wstring name = convertUtf8ToWString(LabelName);
	CAssemblerLabel* Label = new CAssemblerLabel(name,Global.RamPos,Global.Section,false);
	AddAssemblerCommand(Label);
	return true;
}

int fileSize(const std::wstring&& fileName)
{
	struct _stat fileStat; 
	int err = _wstat(fileName.c_str(), &fileStat ); 
	if (0 != err) return 0; 
	return fileStat.st_size; 
}

int fileSize(const std::string&& fileName)
{
	struct _stat fileStat; 
	int err = _stat(fileName.c_str(), &fileStat ); 
	if (0 != err) return 0; 
	return fileStat.st_size; 
}

bool fileExists(const std::wstring& strFilename)
{
	struct _stat stFileInfo;
	int intStat = _wstat(strFilename.c_str(),&stFileInfo);
	return intStat == 0;
}

bool fileExists(const std::string& strFilename)
{
	struct _stat stFileInfo;
	int intStat = _stat(strFilename.c_str(),&stFileInfo);
	return intStat == 0;
}

bool FileExists(char* strFilename)
{
	struct stat stFileInfo;
	int intStat;


	intStat = stat(strFilename,&stFileInfo);
	if(intStat == 0)
	{
		return true;
	} else {
		return false;
	}
}

void AddAssemblerCommand(CAssemblerCommand* Command)
{
	Global.Commands.push_back(Command);
}

void QueueError(eErrorLevel level, char* format, ...)
{
	char str[1024];
	va_list args;

	va_start(args,format);
	vsprintf(str,format,args);
	va_end (args);

	Global.ErrorQueue.AddEntry(level,str);
}

void PrintError(eErrorLevel level, char* format, ...)
{
	char str[1024];
	va_list args;

	va_start(args,format);
	vsprintf(str,format,args);
	va_end (args);

	char* FileName = Global.FileInfo.FileList.GetEntry(Global.FileInfo.FileNum);

	switch (level)
	{
	case ERROR_WARNING:
		printf("%s(%d) warning: %s\n",FileName,Global.FileInfo.LineNumber,str);
		if (Global.warningAsError == true) Global.Error = true;
		break;
	case ERROR_ERROR:
		printf("%s(%d) error: %s\n",FileName,Global.FileInfo.LineNumber,str);
		Global.Error = true;
		break;
	case ERROR_FATALERROR:
		printf("%s(%d) fatal error: %s\n",FileName,Global.FileInfo.LineNumber,str);
		exit(2);
	case ERROR_NOTICE:
		printf("%s(%d) notice: %s\n",FileName,Global.FileInfo.LineNumber,str);
		break;
	}
}


int ReadFileToBuffer(char* FileName, unsigned char** Pointer)
{
#ifdef USE_WINDOWS_FUNCS
	DWORD dwBytesRead;

	HANDLE hSource = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hSource == INVALID_HANDLE_VALUE) return -1;

	int Size = GetFileSize(hSource,NULL);
	*Pointer = (unsigned char*) malloc(Size);
	ReadFile(hSource,*Pointer,Size,&dwBytesRead,NULL);
	CloseHandle(hSource);

	return Size;
#else
	FILE* Input = fopen(FileName,"rb");
	if (Input == NULL) return -1;
	fseek(Input,0,SEEK_END);
	int Size = ftell(Input);
	rewind(Input);
	
	*Pointer = (unsigned char*) malloc(Size);
	fread(*Pointer,1,Size,Input);
	fclose(Input);

	return Size;
#endif
}

void WriteToTempData(FILE*& Output, char* str, int RamPos)
{
	char Dest[2048];
	int pos = sprintf(Dest,"%08X %s",RamPos,str);

	while (pos < 70)
	{
		Dest[pos++] = ' ';
	}

	sprintf(&Dest[pos],"; %s line %d",
		Global.FileInfo.FileList.GetEntry(Global.FileInfo.FileNum),Global.FileInfo.LineNumber);
	fprintf(Output,"%s\n",Dest);
}

void WriteTempFile()
{
	if (Global.TempData.Write == true)
	{
		int FileCount = Global.FileInfo.FileList.GetCount();
		int LineCount = Global.FileInfo.TotalLineCount;
		int LabelCount = Global.symbolTable.getLabelCount();
		int EquCount = Global.symbolTable.getEquationCount();

		FILE* Temp = fopen(Global.TempData.Name,"w");
		fprintf(Temp,"; %d %s included\n",FileCount,FileCount == 1 ? "file" : "files");
		fprintf(Temp,"; %d %s\n",LineCount,LineCount == 1 ? "line" : "lines");
		fprintf(Temp,"; %d %s\n",LabelCount,LabelCount == 1 ? "label" : "labels");
		fprintf(Temp,"; %d %s\n\n",EquCount,EquCount == 1 ? "equation" : "equations");
		for (int i = 0; i < FileCount; i++)
		{
			fprintf(Temp,"; %s\n",Global.FileInfo.FileList.GetEntry(i));
		}

		fprintf(Temp,"\n");

		for (size_t i = 0; i < Global.Commands.size(); i++)
		{
			if (Global.Commands[i]->IsConditional() == false)
			{
				if (Global.ConditionData.EntryCount != 0)
				{
					if (ConditionalAssemblyTrue() == false)
//					if (Global.ConditionData.Entries[Global.ConditionData.EntryCount-1].ConditionTrue == false)
					{
						continue;
					}
				}
			}
			Global.Commands[i]->SetFileInfo();
			Global.Commands[i]->WriteTempData(Temp);
		}
		fclose(Temp);
	}
}


bool isPowerOfTwo(int n)
{
	if (n == 0) return false;
	return !(n & (n - 1));
}

bool CheckBom(FILE*& Handle)
{
	int num = 0;
	fread(&num,2,1,Handle);
	switch (num)
	{
	case 0xFFFE:
	case 0xFEFF:
		PrintError(ERROR_ERROR,"UTF-16 not supported");
		return false;
	case 0xBBEF:
		if (fgetc(Handle) == 0xBF) break;
	default:
		rewind(Handle);
		break;
	}
	return true;
}
