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

		size_t fileCount = Global.FileInfo.FileList.GetCount();
		size_t lineCount = Global.FileInfo.TotalLineCount;
		size_t labelCount = Global.symbolTable.getLabelCount();
		size_t equCount = Global.symbolTable.getEquationCount();

		file.writeFormat(L"; %d %S included\n",fileCount,fileCount == 1 ? "file" : "files");
		file.writeFormat(L"; %d %S\n",lineCount,lineCount == 1 ? "line" : "lines");
		file.writeFormat(L"; %d %S\n",labelCount,labelCount == 1 ? "label" : "labels");
		file.writeFormat(L"; %d %S\n\n",equCount,equCount == 1 ? "equation" : "equations");
		for (size_t i = 0; i < fileCount; i++)
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

void TempData::writeLine(u64 memoryAddress, const std::wstring& text)
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

void CByteList::AddEntry(unsigned char* ByteData, size_t len)
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
