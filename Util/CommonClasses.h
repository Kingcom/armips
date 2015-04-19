#pragma once
#include <vector>
#include "FileClasses.h"

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
