#pragma once

#include "Util/FileSystem.h"

#include <list>
#include <vector>

#include <tinyformat.h>

class TextFile
{
public:
	enum Encoding { ASCII, UTF8, UTF16LE, UTF16BE, SJIS, GUESS };
	enum Mode { Read, Write };
	
	TextFile();
	~TextFile();
	void openMemory(const std::wstring& content);
	bool open(const fs::path& fileName, Mode mode, Encoding defaultEncoding = GUESS);
	bool open(Mode mode, Encoding defaultEncoding = GUESS);
	bool isOpen() { return fromMemory || stream.is_open(); };
	bool atEnd() { return isOpen() && mode == Read && tell() >= size_; };
	long size() { return size_; };
	void close();

	bool hasGuessedEncoding() { return guessedEncoding; };
	bool isFromMemory() { return fromMemory; }
	int getNumLines() { return lineCount; }

	void setFileName(const fs::path& name) { fileName = name; };
	const fs::path& getFileName() { return fileName; };

	wchar_t readCharacter();
	std::wstring readLine();
	std::vector<std::wstring> readAll();
	void writeCharacter(wchar_t character);
	void write(const wchar_t* line);
	void write(const std::wstring& line);
	void write(const char* value);
	void write(const std::string& value);
	void writeLine(const wchar_t* line);
	void writeLine(const std::wstring& line);
	void writeLine(const char* line);
	void writeLine(const std::string& line);
	void writeLines(std::vector<std::wstring>& list);
	
	template <typename... Args>
	void writeFormat(const wchar_t* text, const Args&... args)
	{
		std::wstring message = tfm::format(text,args...);
		write(message);
	}

	bool hasError() { return errorText.size() != 0 && !errorRetrieved; };
	const std::wstring& getErrorText() { errorRetrieved = true; return errorText; };
private:
	long tell();
	void seek(long pos);

	fs::fstream stream;
	fs::path fileName;
	Encoding encoding;
	Mode mode;
	bool recursion;
	bool guessedEncoding;
	long size_;
	std::wstring errorText;
	bool errorRetrieved;
	bool fromMemory;
	std::wstring content;
	size_t contentPos;
	int lineCount;

	std::string buf;
	size_t bufPos;

	inline unsigned char bufGetChar()
	{
		if (buf.size() <= bufPos)
		{
			bufFillRead();
			if (buf.size() == 0)
				return 0;
		}
		return buf[bufPos++];
	}
	inline unsigned short bufGet16LE()
	{
		unsigned char c1 = bufGetChar();
		unsigned char c2 = bufGetChar();
		return c1 | (c2 << 8);
	}
	inline unsigned short bufGet16BE()
	{
		unsigned char c1 = bufGetChar();
		unsigned char c2 = bufGetChar();
		return c2 | (c1 << 8);
	}

	void bufPut(const void *p, const size_t len);
	void bufPut(const char c);

	void bufFillRead();
	void bufDrainWrite();
};

wchar_t sjisToUnicode(unsigned short);
TextFile::Encoding getEncodingFromString(const std::wstring& str);
