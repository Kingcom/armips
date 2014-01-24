#pragma once
#include <vector>
#include "Util/FileClasses.h"

class IElfRelocator;

class CArchitecture
{
public:
	virtual void AssembleOpcode(const std::wstring& name, const std::wstring& args) = 0;
	virtual bool AssembleDirective(const std::wstring& name, const std::wstring& args) = 0;
	virtual void NextSection() = 0;
	virtual void Pass2() = 0;
	virtual void Revalidate() = 0;
	virtual int GetWordSize() = 0;
	virtual IElfRelocator* getElfRelocator() = 0;
};

class CInvalidArchitecture: public CArchitecture
{
public:
	virtual void AssembleOpcode(const std::wstring& name, const std::wstring& args);
	virtual bool AssembleDirective(const std::wstring& name, const std::wstring& args);
	virtual void NextSection();
	virtual void Pass2();
	virtual void Revalidate();
	virtual int GetWordSize();
	virtual IElfRelocator* getElfRelocator();
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

class TempData
{
public:
	void setFileName(const std::wstring& name) { file.setFileName(name); };
	void clear() { file.setFileName(L""); }
	void start();
	void end();
	void writeLine(int memoryAddress, const std::wstring& text);
private:
	TextFile file;
};

class IntegerStack
{
public:
	void push(unsigned int num) { stack.push_back(num); };
	unsigned int pop() { unsigned int value = stack.back(); stack.pop_back(); return value; };
	bool isEmpty() { return stack.size() == 0; };
	int size() { return stack.size(); };
	void clear() { stack.clear(); };
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