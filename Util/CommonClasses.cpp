#include "stdafx.h"
#include "Util/CommonClasses.h"
#include "Core/Common.h"
#include "Core/Directives.h"

CInvalidArchitecture InvalidArchitecture;

void CInvalidArchitecture::AssembleOpcode(char* name, char* args)
{
	PrintError(ERROR_FATALERROR,"No architecture specified");
}

bool CInvalidArchitecture::AssembleDirective(const std::wstring& name, const std::wstring& args)
{
	return directiveAssembleGlobal(name,args);
}

void CInvalidArchitecture::NextSection()
{
	PrintError(ERROR_FATALERROR,"No architecture specified");
}

void CInvalidArchitecture::Pass2()
{
	PrintError(ERROR_FATALERROR,"No architecture specified");
}

void CInvalidArchitecture::Revalidate()
{
	PrintError(ERROR_FATALERROR,"No architecture specified");
}

int CInvalidArchitecture::GetFileAddress(int MemoryAddress)
{
	PrintError(ERROR_FATALERROR,"No architecture specified");
	return 0;
}

int CInvalidArchitecture::GetMemoryAddress(int FileAddress)
{
	PrintError(ERROR_FATALERROR,"No architecture specified");
	return 0;
}

int CInvalidArchitecture::GetWordSize()
{
	PrintError(ERROR_FATALERROR,"No architecture specified");
	return 0;
}


void TempData::start()
{
	if (file.getFileName().empty() == false)
	{
		if (file.open(TextFile::Write) == false)
		{
			PrintError(ERROR_ERROR,"Could not open temp file %ls.",file.getFileName().c_str());
			return;
		}

		int fileCount = Global.FileInfo.FileList.GetCount();
		int lineCount = Global.FileInfo.TotalLineCount;
		int labelCount = Global.symbolTable.getLabelCount();
		int equCount = Global.symbolTable.getEquationCount();

		file.writeFormat(L"; %d %S included\n",fileCount,fileCount == 1 ? "file" : "files");
		file.writeFormat(L"; %d %S\n",lineCount,lineCount == 1 ? "line" : "lines");
		file.writeFormat(L"; %d %S\n",labelCount,labelCount == 1 ? "label" : "labels");
		file.writeFormat(L"; %d %S\n\n",equCount,equCount == 1 ? "equation" : "equations");
		for (int i = 0; i < fileCount; i++)
		{
			file.writeFormat(L"; %S\n",Global.FileInfo.FileList.GetEntry(i));
		}
		file.writeLine("");
	}
}

void TempData::end()
{
	if (file.isOpen())
		file.close();
}

void TempData::writeLine(int memoryAddress, const std::wstring& text)
{
	if (file.isOpen())
	{
		std::wstring str = formatString(L"%08X %ls",memoryAddress,text.c_str());
		while (str.size() < 70)
			str += ' ';

		str += formatString(L"; %S line %d",
			Global.FileInfo.FileList.GetEntry(Global.FileInfo.FileNum),Global.FileInfo.LineNumber);

		file.writeLine(str);
	}
}


void SymbolData::start()
{
	if (file.getFileName().empty() == false)
	{
		if (file.open(TextFile::Write,TextFile::ASCII) == false)
		{
			PrintError(ERROR_ERROR,"Could not open sym file %ls.",file.getFileName().c_str());
			return;
		}
		file.writeLine(L"00000000 0");
	}
}

void SymbolData::end()
{
	if (file.isOpen())
	{
		file.write("\x1A");
		file.close();
	}
}

void SymbolData::addSymbol(int memoryAddress, const std::wstring& name)
{
	if (file.isOpen() && enabled)
	{
		file.writeFormat(L"%08X %ls\n",memoryAddress,name.c_str());
	}
}


CStringList::CStringList()
{
	EntryPoses = (int*) malloc(256*4);
	EntryCount = 0;
	EntriesAllocated = 256;
	Data = (char*) malloc(1024);
	DataPos = 0;
	DataAllocated = 1024;
}

CStringList::~CStringList()
{
	free(EntryPoses);
	free(Data);
}

void CStringList::AddEntry(char* str)
{
	int len = strlen(str)+1;
	if (EntriesAllocated == EntryCount)
	{
		EntriesAllocated <<= 1;
		EntryPoses = (int*) realloc(EntryPoses,EntriesAllocated*4);
	}
	if (DataPos+len > DataAllocated)
	{
		DataAllocated <<= 1;
		Data = (char*) realloc(Data,DataAllocated);
	}

	EntryPoses[EntryCount++] = DataPos;
	memcpy(&Data[DataPos],str,len);
	DataPos += len;
}

char* CStringList::GetEntry(int num)
{
	if (num >= EntryCount) return NULL;
	return &Data[EntryPoses[num]];
}


CErrorQueue::CErrorQueue()
{
	Entries = (tErrorQueueEntry*) malloc(256*sizeof(tErrorQueueEntry));
	EntryCount = 0;
	EntriesAllocated = 256;
	Data = (char*) malloc(1024);
	DataPos = 0;
	DataAllocated = 1024;
}

CErrorQueue::~CErrorQueue()
{
	free(Entries);
	free(Data);
}

void CErrorQueue::AddEntry(int Level, char* str)
{
	int len = strlen(str)+1;
	if (EntriesAllocated == EntryCount)
	{
		EntriesAllocated <<= 1;
		Entries = (tErrorQueueEntry*) realloc(Entries,EntriesAllocated*sizeof(tErrorQueueEntry));
	}
	if (DataPos+len > DataAllocated)
	{
		DataAllocated <<= 1;
		Data = (char*) realloc(Data,DataAllocated);
	}

	Entries[EntryCount].Pos = DataPos;
	Entries[EntryCount].FileNum = Global.FileInfo.FileNum;
	Entries[EntryCount].Level = Level;
	Entries[EntryCount++].LineNum = Global.FileInfo.LineNumber;
	memcpy(&Data[DataPos],str,len);
	DataPos += len;
}

void CErrorQueue::Output()
{
	for (int i = 0; i < EntryCount; i++)
	{
		Global.FileInfo.FileNum = Entries[i].FileNum;
		Global.FileInfo.LineNumber = Entries[i].LineNum;
		PrintError((eErrorLevel)Entries[i].Level,&Data[Entries[i].Pos]);
	}
}



CByteList::CByteList()
{
	Entries = (tByteListEntry*) malloc(256*sizeof(tByteListEntry));
	EntryCount = 0;
	EntriesAllocated = 256;
	Data = (unsigned char*) malloc(1024);
	DataPos = 0;
	DataAllocated = 1024;
}

CByteList::~CByteList()
{
	free(Entries);
	free(Data);
}

void CByteList::AddEntry(unsigned char* ByteData, int len)
{
	if (EntriesAllocated == EntryCount)
	{
		EntriesAllocated <<= 1;
		Entries = (tByteListEntry*) realloc(Entries,EntriesAllocated*sizeof(tByteListEntry));
	}
	if (DataPos+len > DataAllocated)
	{
		DataAllocated <<= 1;
		Data = (unsigned char*) realloc(Data,DataAllocated);
	}

	Entries[EntryCount].Pos = DataPos;
	Entries[EntryCount++].len = len;
	memcpy(&Data[DataPos],ByteData,len);
	DataPos += len;
}
