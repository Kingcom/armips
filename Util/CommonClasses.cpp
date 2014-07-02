#include "stdafx.h"
#include "Util/CommonClasses.h"
#include "Core/Common.h"

void TempData::start()
{
	if (file.getFileName().empty() == false)
	{
		if (file.open(TextFile::Write) == false)
		{
			Logger::printError(Logger::Error,L"Could not open temp file %s.",file.getFileName());
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
		std::wstring str = formatString(L"%08X %s",memoryAddress,text);
		while (str.size() < 70)
			str += ' ';

		str += formatString(L"; %S line %d",
			Global.FileInfo.FileList.GetEntry(Global.FileInfo.FileNum),Global.FileInfo.LineNumber);

		file.writeLine(str);
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
