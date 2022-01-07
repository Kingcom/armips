#pragma once

#include "Util/ByteArray.h"
#include "Util/FileClasses.h"

#include <map>
#include <vector>

class Trie
{
public:
	Trie();
	void insert(const char* text, size_t value);
	bool findLongestPrefix(const char* text, size_t& result);
private:
	struct LookupEntry
	{
		size_t node;
		char input;

		bool operator<(const LookupEntry& other) const
		{
			if (node != other.node)
				return node < other.node;
			return input < other.input;
		}
	};

	struct Node
	{
		size_t index;
		bool hasValue;
		size_t value;
	};

	std::vector<Node> nodes;
	std::map<LookupEntry,size_t> lookup;
};

class EncodingTable
{
public:
	EncodingTable();
	~EncodingTable();
	void clear();
	bool load(const fs::path& fileName, TextFile::Encoding encoding = TextFile::GUESS);
	bool isLoaded() { return entries.size() != 0; };
	void addEntry(unsigned char* hex, size_t hexLength, const std::string& value);
	void setTerminationEntry(unsigned char* hex, size_t hexLength);
	ByteArray encodeString(const std::string& str, bool writeTermination = true);
	ByteArray encodeTermination();
private:
	struct TableEntry
	{
		size_t hexPos;
		size_t hexLen;
		size_t valueLen;
	};

	ByteArray hexData;
	std::vector<TableEntry> entries;
	Trie lookup;
	TableEntry terminationEntry;
};
