#pragma once

#include "Util/ByteArray.h"
#include "Util/FileClasses.h"

#include <memory>
#include <vector>

class SymbolData;

struct SymDataModuleInfo;

class AssemblerFile
{
public:
	virtual ~AssemblerFile() { };
	
	virtual bool open(bool onlyCheck) = 0;
	virtual void close() = 0;
	virtual bool isOpen() = 0;
	virtual bool write(void* data, size_t length) = 0;
	virtual int64_t getVirtualAddress() = 0;
	virtual int64_t getPhysicalAddress() = 0;
	virtual int64_t getHeaderSize() = 0;
	virtual bool seekVirtual(int64_t virtualAddress) = 0;
	virtual bool seekPhysical(int64_t physicalAddress) = 0;
	virtual bool getModuleInfo(SymDataModuleInfo& info) { return false; };
	virtual bool hasFixedVirtualAddress() { return false; };
	virtual void beginSymData(SymbolData& symData) { };
	virtual void endSymData(SymbolData& symData) { };
	virtual const fs::path& getFileName() = 0;
};

class GenericAssemblerFile: public AssemblerFile
{
public:
	GenericAssemblerFile(const fs::path& fileName, int64_t headerSize, bool overwrite);
	GenericAssemblerFile(const fs::path& fileName, const fs::path& originalFileName, int64_t headerSize);

	virtual bool open(bool onlyCheck);
	virtual void close() { if (stream.is_open()) stream.close(); };
	virtual bool isOpen() { return stream.is_open(); };
	virtual bool write(void* data, size_t length);
	virtual int64_t getVirtualAddress() { return virtualAddress; };
	virtual int64_t getPhysicalAddress() { return virtualAddress-headerSize; };
	virtual int64_t getHeaderSize() { return headerSize; };
	virtual bool seekVirtual(int64_t virtualAddress);
	virtual bool seekPhysical(int64_t physicalAddress);
	virtual bool hasFixedVirtualAddress() { return true; };

	virtual const fs::path& getFileName() { return fileName; };
	const fs::path& getOriginalFileName() { return originalName; };
	int64_t getOriginalHeaderSize() { return originalHeaderSize; };
	void setHeaderSize(int64_t size) { headerSize = size; };

private:
	enum Mode { Open, Create, Copy };

	Mode mode;
	int64_t originalHeaderSize;
	int64_t headerSize;
	int64_t virtualAddress;
	fs::ofstream stream;
	fs::path fileName;
	fs::path originalName;
};


class FileManager
{
public:
	FileManager();
	~FileManager();
	void reset();
	bool openFile(std::shared_ptr<AssemblerFile> file, bool onlyCheck);
	void addFile(std::shared_ptr<AssemblerFile> file);
	bool hasOpenFile() { return activeFile != nullptr; };
	void closeFile();
	bool write(void* data, size_t length);
	bool writeU8(uint8_t data);
	bool writeU16(uint16_t data);
	bool writeU32(uint32_t data);
	bool writeU64(uint64_t data);
	int64_t getVirtualAddress();
	int64_t getPhysicalAddress();
	int64_t getHeaderSize();
	bool seekVirtual(int64_t virtualAddress);
	bool seekPhysical(int64_t physicalAddress);
	bool advanceMemory(size_t bytes);
	std::shared_ptr<AssemblerFile> getOpenFile() { return activeFile; };
	int64_t getOpenFileID();
	void setEndianness(Endianness endianness) { this->endianness = endianness; };
	Endianness getEndianness() { return endianness; }
private:
	bool checkActiveFile();
	std::vector<std::shared_ptr<AssemblerFile>> files;
	std::shared_ptr<AssemblerFile> activeFile;
	Endianness endianness;
	Endianness ownEndianness;
};
