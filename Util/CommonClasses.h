#pragma once
#include <vector>
#include "FileClasses.h"

class CStringList
{
public:
	void Clear() { data.clear(); };
	void AddEntry(char* str) { data.push_back(str); };
	char* GetEntry(size_t num) { return (char*) data[num].c_str(); };
	size_t GetCount() { return data.size(); };
private:
	std::vector<std::string> data;
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

	const ArgumentList::Entry& operator [](size_t index) const
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
	void writeLine(u64 memoryAddress, const std::wstring& text);
private:
	TextFile file;
};

class IntegerStack
{
public:
	void push(unsigned int num) { stack.push_back(num); };
	unsigned int pop() { unsigned int value = stack.back(); stack.pop_back(); return value; };
	bool isEmpty() { return stack.size() == 0; };
	size_t size() { return stack.size(); };
	void clear() { stack.clear(); };
private:
	std::vector<unsigned int> stack;
};


typedef struct {
	size_t Pos;
	size_t len;
} tByteListEntry;

class CByteList
{
public:
	CByteList();
	~CByteList();
	void Clear() { EntryCount = 0; DataPos = 0; };
	void AddEntry(unsigned char* ByteData, size_t len);
	unsigned char* GetEntry(size_t num) { return &Data[Entries[num].Pos]; };
	size_t GetLen(size_t num) { return Entries[num].len; };
	size_t GetCount() { return EntryCount; };
private:
	tByteListEntry* Entries;
	size_t EntryCount;
	size_t EntriesAllocated;
	unsigned char* Data;
	size_t DataPos;
	size_t DataAllocated;
};