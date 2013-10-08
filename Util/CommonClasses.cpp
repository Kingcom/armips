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
