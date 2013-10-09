#pragma once
#include <list>
#include "Util/Util.h"

class BinaryFile
{
public:
	enum Mode { Read, Write, ReadWrite };

	BinaryFile();
	~BinaryFile();

	bool open(const std::wstring& fileName, Mode mode);
	bool isOpen() { return handle != NULL; };
	bool atEnd() { return isOpen() && mode != Write && ftell(handle) == size_; };
	void setPos(long pos) { if (isOpen()) fseek(handle,pos,SEEK_SET); };
	long pos() { return isOpen() ? ftell(handle) : -1; }
	long size() { return size_; };
	void close();

	int read(void* dest, int length);
	int write(void* source, int length);
private:
	FILE* handle;
	Mode mode;
	long size_;
};


class TextFile
{
public:
	enum Encoding { UTF8, UTF16LE, UTF16BE, SJIS };
	enum Mode { Read, Write };
	
	TextFile();
	~TextFile();
	bool open(const std::wstring& fileName, Mode mode, Encoding defaultEncoding = UTF8);
	bool open(Mode mode, Encoding defaultEncoding = UTF8);
	bool isOpen() { return handle != NULL; };
	bool atEnd() { return isOpen() && mode == Read && ftell(handle) == size_; };
	long size() { return size_; };
	void close();

	void setFileName(const std::wstring& name) { fileName = name; };
	const std::wstring& getFileName() { return fileName; };

	wchar_t readCharacter();
	std::wstring readLine();
	StringList readAll();
	void writeCharacter(wchar_t character);
	void write(const wchar_t* line);
	void write(const std::wstring& line);
	void write(const char* value);
	void write(const std::string& value);
	void writeLine(const wchar_t* line);
	void writeLine(const std::wstring& line);
	void writeLine(const char* line);
	void writeLine(const std::string& line);
	void writeLines(StringList& list);
	void writeFormat(wchar_t* format, ...);
private:
	FILE* handle;
	std::wstring fileName;
	Encoding encoding;
	Mode mode;
	bool recursion;
	long size_;
};
