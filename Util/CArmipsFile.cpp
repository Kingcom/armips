#include "stdafx.h"
#include "Util/CArmipsFile.h"
#include "Core/Common.h"


CArmipsFile::CArmipsFile()
{
#ifdef CAF_USEHANDLES
	hFile = INVALID_HANDLE_VALUE;
#elif defined(CAF_USEIOBUF)
	File = NULL;
#endif
}

CArmipsFile::~CArmipsFile()
{
#ifdef CAF_USEHANDLES
	if (hFile != INVALID_HANDLE_VALUE)
	{
		PrintError(ERROR_WARNING,"File not closed");
		CloseHandle(hFile);
	}
#elif defined(CAF_USEIOBUF)
	if (File != NULL)
	{
		PrintError(ERROR_WARNING,"File not closed");
		fclose(File);
	}
#endif
}

bool CArmipsFile::Opened()
{
#ifdef CAF_USEHANDLES
	return (hFile != INVALID_HANDLE_VALUE);
#elif defined(CAF_USEIOBUF)
	return (File != NULL);
#endif
}

bool CArmipsFile::Open(char* FileName)
{
	if (Opened()) Close();
#ifdef CAF_USEHANDLES
	hFile = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		FileSize = SetFilePointer(hFile,0,NULL,FILE_END);
		SetFilePointer(hFile,0,NULL,FILE_BEGIN);
	}
#elif defined(CAF_USEIOBUF)
	File = fopen(FileName,"rb+");
	if (File != NULL)
	{
		fseek(File,0,SEEK_END);
		FileSize = ftell(File);
		rewind(File);
	}
#endif
	return Opened();
}

bool CArmipsFile::Create(char* FileName)
{
	if (Opened()) Close();
#ifdef CAF_USEHANDLES
	hFile = CreateFile(FileName, GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#elif defined(CAF_USEIOBUF)
	File = fopen(FileName,"wb");
#endif
	FileSize = 0;
	return Opened();
}

void CArmipsFile::Close()
{
	if (Opened())
	{
#ifdef CAF_USEHANDLES
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
#elif defined(CAF_USEIOBUF)
		fclose(File);
		File = NULL;
#endif
	}
}

int CArmipsFile::Seek(int pos)
{
	if (Opened() == false) return -1;
#ifdef CAF_USEHANDLES
	return SetFilePointer(hFile,pos,NULL,FILE_BEGIN);
#elif defined(CAF_USEIOBUF)
	return fseek(File,pos,SEEK_SET);
#endif
}

int CArmipsFile::Tell()
{
	if (Opened() == false) return -1;
#ifdef CAF_USEHANDLES
	return SetFilePointer(hFile,0,NULL,FILE_CURRENT);
#elif defined(CAF_USEIOBUF)
	return ftell(File);
#endif
}

int CArmipsFile::Size()
{
	if (Opened() == false) return -1;
	return FileSize;
}

int CArmipsFile::Write(void* Data, int len)
{
	if (Opened() == false) return -1;
#ifdef CAF_USEHANDLES
	DWORD dwBytesWritten;
	WriteFile(hFile,Data,len,&dwBytesWritten,NULL);
	return dwBytesWritten;
#elif defined(CAF_USEIOBUF)
	return (int) fwrite(Data,1,len,File);
#endif
}
