#pragma once
#include <vector>

class CArchitecture
{
public:
	virtual void AssembleOpcode(char* name, char* args) = 0;
	virtual bool AssembleDirective(char* name, char* args) = 0;
	virtual void NextSection() = 0;
	virtual void Pass2() = 0;
	virtual void Revalidate() = 0;
	virtual int GetFileAddress(int MemoryAddress) = 0;
	virtual int GetMemoryAddress(int FileAddress) = 0;
	virtual int GetWordSize() = 0;
};

class CInvalidArchitecture: public CArchitecture
{
public:
	virtual void AssembleOpcode(char* name, char* args);
	virtual bool AssembleDirective(char* name, char* args);
	virtual void NextSection();
	virtual void Pass2();
	virtual void Revalidate();
	virtual int GetFileAddress(int MemoryAddress);
	virtual int GetMemoryAddress(int FileAddress);
	virtual int GetWordSize();
};

extern CInvalidArchitecture InvalidArchitecture;

class CStringList
{
public:
	CStringList();
	~CStringList();
	void Clear() { EntryCount = 0; DataPos = 0; };
	void AddEntry(char* str);
	char* GetEntry(int num);
	int GetCount() { return EntryCount; };
private:
	int* EntryPoses;
	int EntryCount;
	int EntriesAllocated;
	char* Data;
	int DataPos;
	int DataAllocated;
};

typedef struct {
	int Pos;
	bool String;
} tArgumentListEntry;

class CArgumentList
{
public:
	CArgumentList();
	~CArgumentList();
	void Clear() { EntryCount = 0; DataPos = 0; };
	void AddEntry(char* str, bool String);
	char* GetEntry(int num);
	bool IsString(int num) { return Entries[num].String; };
	int GetCount() { return EntryCount; };
private:
	tArgumentListEntry* Entries;
	int EntryCount;
	int EntriesAllocated;
	char* Data;
	int DataPos;
	int DataAllocated;
};

class ArgumentList
{
public:
	struct Entry
	{
		std::wstring text;
		bool isString;
	};

	void add(const std::wstring& text, bool isString)
	{
		Entry entry = { text, isString };
		entries.push_back(entry);
	};

	const ArgumentList::Entry& operator [](unsigned int index) const
	{
		return entries[index];
	};

	size_t size() const { return entries.size(); };
	void clear() { entries.clear(); };
private:
	std::vector<Entry> entries;
};

typedef struct {
	int Pos;
	int FileNum;
	int LineNum;
	int Level;
} tErrorQueueEntry;

class CErrorQueue
{
public:
	CErrorQueue();
	~CErrorQueue();
	void Clear() { EntryCount = 0; DataPos = 0; };
	void AddEntry(int Level, char* str);
	void Output();
	bool isEmpty() { return EntryCount == 0; };
private:
	tErrorQueueEntry* Entries;
	int EntryCount;
	int EntriesAllocated;
	char* Data;
	int DataPos;
	int DataAllocated;
};


class IntegerStack
{
public:
	void Push(unsigned int num) { stack.push_back(num); };
	unsigned int Pop() { unsigned int value = stack[stack.size()-1]; stack.pop_back(); return value; };
	bool IsEmpty() { return stack.size() == 0; };
	int GetCount() { return stack.size(); };
private:
	std::vector<unsigned int> stack;
};


typedef struct {
	int Pos;
	int len;
} tByteListEntry;

class CByteList
{
public:
	CByteList();
	~CByteList();
	void Clear() { EntryCount = 0; DataPos = 0; };
	void AddEntry(unsigned char* ByteData, int len);
	unsigned char* GetEntry(int num) { return &Data[Entries[num].Pos]; };
	int GetLen(int num) { return Entries[num].len; };
	int GetCount() { return EntryCount; };
private:
	tByteListEntry* Entries;
	int EntryCount;
	int EntriesAllocated;
	unsigned char* Data;
	int DataPos;
	int DataAllocated;
};