#include "stdafx.h"
#include "Core/Common.h"
#include <sys/stat.h>
#include "Assembler.h"
#include "Commands/CAssemblerLabel.h"
#include "Util/Util.h"

tGlobal Global;
CArchitecture* Arch;
#include <direct.h>

std::wstring getFolderNameFromPath(const std::wstring& src)
{
	size_t s = src.rfind('\\');
	if (s == std::wstring::npos)
	{
		s = src.rfind('/');
		if (s == std::wstring::npos)
			return L".";
	}

	return src.substr(0,s);
}

std::wstring getFullPathName(const std::wstring& path)
{
	if (Global.relativeInclude == true)
	{
		if (path.size() >= 3 && path[1] == ':' && (path[2] == '/' || path[2] == '\\'))
		{
			return path;
		} else {
			std::wstring source = convertUtf8ToWString(Global.FileInfo.FileList.GetEntry(Global.FileInfo.FileNum));
			return getFolderNameFromPath(source) + L"/" + path;
		}
	} else {
		return path;
	}
}

bool checkLabelDefined(const std::wstring& labelName)
{
	Label* label = Global.symbolTable.getLabel(labelName,Global.FileInfo.FileNum,Global.Section);
	return label->isDefined();
}

bool checkValidLabelName(const std::wstring& labelName)
{
	return Global.symbolTable.isValidSymbolName(labelName);
}

bool addAssemblerLabel(const std::wstring& labelName)
{
	if (checkValidLabelName(labelName) == false)
	{
		PrintError(ERROR_ERROR,"Invalid label name \"%ls\"",labelName.c_str());
		return false;
	}

	if (checkLabelDefined(labelName) == true)
	{
		PrintError(ERROR_ERROR,"Label \"%ls\" already defined",labelName.c_str());
		return false;
	}

	CAssemblerLabel* Label = new CAssemblerLabel(labelName,Global.RamPos,Global.Section,false);
	AddAssemblerCommand(Label);
	return true;
}

int fileSize(const std::wstring& fileName)
{
	struct _stat fileStat; 
	int err = _wstat(fileName.c_str(), &fileStat ); 
	if (0 != err) return 0; 
	return fileStat.st_size; 
}

int fileSize(const std::string& fileName)
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

	HANDLE hSource = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ,
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
