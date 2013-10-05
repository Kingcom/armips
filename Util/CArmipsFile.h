#pragma once

#if 0			// Use Handles
#define CAF_USEHANDLES
#undef CAF_USEIOBUF
#else			// Use Iobuf
#undef CAF_USEHANDLES
#define CAF_USEIOBUF
#endif

class CArmipsFile
{
public:
	CArmipsFile();
	~CArmipsFile();
	bool Opened();
	bool Open(char* FileName);
	bool Create(char* FileName);
	void Close();
	int Seek(int pos);
	int Tell();
	int Size();
	int Write(void* Data, int len);
private:
#ifdef CAF_USEHANDLES
	HANDLE hFile;
#elif defined(CAF_USEIOBUF)
	FILE* File;
#endif
	int FileSize;
};
