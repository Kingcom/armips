#pragma once
#include <vector>
#include "Util/FileClasses.h"

class AssemblerFile
{
public:
	
	virtual bool open(bool onlyCheck) = 0;
	virtual void close() = 0;
	virtual bool isOpen() = 0;
	virtual bool write(void* data, int length) = 0;
	virtual int getVirtualAddress() = 0;
	virtual int getPhysicalAddress() = 0;
	virtual bool seekVirtual(int virtualAddress) = 0;
	virtual bool seekPhysical(int physicalAddress) = 0;
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
	virtual int getVirtualAddress() { return virtualAddress; };
	virtual int getPhysicalAddress() { return virtualAddress-headerSize; };
	virtual bool seekVirtual(int virtualAddress);
	virtual bool seekPhysical(int physicalAddress);

	const std::wstring& getFileName() { return fileName; };
	const std::wstring& getOriginalFileName() { return originalName; };
	int getOriginalHeaderSize() { return originalHeaderSize; };
	void setHeaderSize(int size) { headerSize = size; };
private:
	enum Mode { Open, Create, Copy };

	Mode mode;
	int originalHeaderSize;
	int headerSize;
	int virtualAddress;
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
	int getVirtualAddress();
	int getPhysicalAddress();
	bool seekVirtual(int virtualAddress);
	bool seekPhysical(int physicalAddress);
	bool advanceMemory(int bytes);
	AssemblerFile* getOpenFile() { return activeFile; };
private:
	bool checkActiveFile();
	std::vector<AssemblerFile*> files;
	AssemblerFile* activeFile;
};