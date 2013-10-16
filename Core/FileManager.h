#pragma once
#include <vector>
#include "Util/FileClasses.h"
#include "Core/SymbolData.h"

class AssemblerFile
{
public:
	
	virtual bool open(bool onlyCheck) = 0;
	virtual void close() = 0;
	virtual bool isOpen() = 0;
	virtual bool write(void* data, int length) = 0;
	virtual size_t getVirtualAddress() = 0;
	virtual size_t getPhysicalAddress() = 0;
	virtual bool seekVirtual(size_t virtualAddress) = 0;
	virtual bool seekPhysical(size_t physicalAddress) = 0;
	virtual bool getModuleInfo(SymDataModuleInfo& info) { return false; };
};

class GenericAssemblerFile: public AssemblerFile
{
public:
	GenericAssemblerFile(const std::wstring& fileName, int headerSize, bool overwrite);
	GenericAssemblerFile(const std::wstring& fileName, const std::wstring& originalFileName, int headerSize);

	virtual bool open(bool onlyCheck);
	virtual void close() { handle.close(); };
	virtual bool isOpen() { return handle.isOpen(); };
	virtual bool write(void* data, int length);
	virtual size_t getVirtualAddress() { return virtualAddress; };
	virtual size_t getPhysicalAddress() { return virtualAddress-headerSize; };
	virtual bool seekVirtual(size_t virtualAddress);
	virtual bool seekPhysical(size_t physicalAddress);

	const std::wstring& getFileName() { return fileName; };
	const std::wstring& getOriginalFileName() { return originalName; };
	int getOriginalHeaderSize() { return originalHeaderSize; };
	void setHeaderSize(int size) { headerSize = size; };
private:
	enum Mode { Open, Create, Copy };

	Mode mode;
	int originalHeaderSize;
	int headerSize;
	size_t virtualAddress;
	BinaryFile handle;
	std::wstring fileName;
	std::wstring originalName;
};


class FileManager
{
public:
	FileManager();
	~FileManager();
	void reset();
	bool openFile(AssemblerFile* file, bool onlyCheck);
	void addFile(AssemblerFile* file);
	bool hasOpenFile() { return activeFile != NULL; };
	void closeFile();
	bool write(void* data, int length);
	size_t getVirtualAddress();
	size_t getPhysicalAddress();
	bool seekVirtual(size_t virtualAddress);
	bool seekPhysical(size_t physicalAddress);
	bool advanceMemory(int bytes);
	AssemblerFile* getOpenFile() { return activeFile; };
private:
	bool checkActiveFile();
	std::vector<AssemblerFile*> files;
	AssemblerFile* activeFile;
};