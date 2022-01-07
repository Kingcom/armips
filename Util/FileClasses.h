#pragma once

#include "Util/FileSystem.h"

#include <list>
#include <optional>
#include <vector>

#include <tinyformat.h>

class TextFile
{
public:
	enum Encoding { ASCII, UTF8, UTF16LE, UTF16BE, SJIS, GUESS };
	enum Mode { Read, Write };
	
	TextFile();
	~TextFile();
	void openMemory(const std::string& content);
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

	std::string readLine();
	std::vector<std::string> readAll();
	void write(const char* value);
	void write(const std::string& value);
	void writeLine(const char* line);
	void writeLine(const std::string& line);
	void writeLines(std::vector<std::string>& list);
	
	template <typename... Args>
	void writeFormat(const char* text, const Args&... args)
	{
		std::string message = tfm::format(text,args...);
		write(message);
	}

	bool hasError() { return errorText.size() != 0 && !errorRetrieved; };
	const std::string& getErrorText() { errorRetrieved = true; return errorText; };
private:
	char32_t readCharacter();

	std::string readLineUtf8();
	std::string readLineSJIS();
	long tell();
	void seek(long pos);

	fs::fstream stream;
	fs::path fileName;
	Encoding encoding;
	Mode mode;
	bool recursion;
	bool guessedEncoding;
	long size_;
	std::string errorText;
	bool errorRetrieved;
	bool fromMemory;
	std::string content;
	size_t contentPos;
	int lineCount;

	std::string buf;
	size_t bufPos;

	inline unsigned char bufGetChar()
	{
		if (fromMemory)
			return content[contentPos++];

		if (buf.size() <= bufPos)
		{
			bufFillRead();
			if (buf.size() == 0)
				return 0;
		}
		++contentPos;
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

std::optional<char16_t> sjisToUnicode(unsigned short);
TextFile::Encoding getEncodingFromString(const std::string& str);
