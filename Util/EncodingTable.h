#pragma once
#include "Util/ByteArray.h"
#include "Util/FileClasses.h"

class EncodingTable
{
public:
	EncodingTable();
	~EncodingTable();
	void clear();
	bool load(const std::wstring& fileName, TextFile::Encoding encoding = TextFile::GUESS);
	bool isLoaded() { return entries.size() != 0; };
	void addEntry(unsigned char* hex, size_t hexLength, const std::wstring& value);
	void addEntry(unsigned char* hex, size_t hexLength, wchar_t value);
	void setTerminationEntry(unsigned char* hex, size_t hexLength);
	ByteArray encodeString(const std::wstring& str, bool writeTermination = true);
	ByteArray encodeTermination();
private:
	int searchStringMatch(const std::wstring& str, size_t pos = 0);

	struct TableEntry
	{
		size_t hexPos;
		size_t hexLen;
		size_t valuePos;
		size_t valueLen;
	};

	ByteArray hexData;
	std::wstring valueData;
	std::vector<TableEntry> entries;
	TableEntry terminationEntry;
};
