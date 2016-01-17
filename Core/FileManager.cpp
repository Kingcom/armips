#include "stdafx.h"
#include "FileManager.h"
#include "Misc.h"
#include "Common.h"

inline u32 swapEndianness32(u32 value)
{
	return ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) | ((value & 0xFF0000) >> 8) | ((value & 0xFF000000) >> 24);
}

inline u16 swapEndianness16(u16 value)
{
	return ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);
}


GenericAssemblerFile::GenericAssemblerFile(const std::wstring& fileName, u32 headerSize, bool overwrite)
{
	this->fileName = fileName;
	this->headerSize = headerSize;
	this->originalHeaderSize = headerSize;
	this->virtualAddress = headerSize;
	mode = overwrite == true ? Create : Open;
}

GenericAssemblerFile::GenericAssemblerFile(const std::wstring& fileName, const std::wstring& originalFileName, u32 headerSize)
{
	this->fileName = fileName;
	this->originalName = originalFileName;
	this->headerSize = headerSize;
	this->virtualAddress = headerSize;
	originalHeaderSize = headerSize;
	mode = Copy;
}

bool GenericAssemblerFile::open(bool onlyCheck)
{
	headerSize = originalHeaderSize;
	virtualAddress = headerSize;

	if (onlyCheck == false)
	{
		// actually open the file
		bool success;
		switch (mode)
		{
		case Open:
			success = handle.open(fileName,BinaryFile::ReadWrite);
			if (success == false)
			{
				Logger::printError(Logger::FatalError,L"Could not open file %s",fileName);
				return false;
			}
			return true;

		case Create:
			success = handle.open(fileName,BinaryFile::Write);
			if (success == false)
			{
				Logger::printError(Logger::FatalError,L"Could not create file %s",fileName);
				return false;
			}
			return true;

		case Copy:
			success = copyFile(originalName,fileName);
			if (success == false)
			{
				Logger::printError(Logger::FatalError,L"Could not copy file %s",originalName);
				return false;
			}

			success = handle.open(fileName,BinaryFile::ReadWrite);
			if (success == false)
			{
				Logger::printError(Logger::FatalError,L"Could not open file %s",fileName);
				return false;
			}
			return true;

		default:
			return false;
		}
	}

	// else only check if it can be done, don't actually do it permanently
	bool success, exists;
	BinaryFile temp;
	switch (mode)
	{
	case Open:
		success = temp.open(fileName,BinaryFile::ReadWrite);
		if (success == false)
		{
			Logger::queueError(Logger::FatalError,L"Could not open file %s",fileName);
			return false;
		}
		temp.close();
		return true;

	case Create:
		// if it exists, check if you can open it with read/write access
		// otherwise open it with write access and remove it afterwards
		exists = fileExists(fileName);
		success = temp.open(fileName,exists ? BinaryFile::ReadWrite : BinaryFile::Write);
		if (success == false)
		{
			Logger::queueError(Logger::FatalError,L"Could not create file %s",fileName);
			return false;
		}
		temp.close();

		if (exists == false)
			deleteFile(fileName);

		return true;

	case Copy:
		// check original file
		success = temp.open(originalName,BinaryFile::ReadWrite);
		if (success == false)
		{
			Logger::queueError(Logger::FatalError,L"Could not open file %s",originalName);
			return false;
		}
		temp.close();

		// check new file, same as create
		exists = fileExists(fileName);
		success = temp.open(fileName,exists ? BinaryFile::ReadWrite : BinaryFile::Write);
		if (success == false)
		{
			Logger::queueError(Logger::FatalError,L"Could not open file %s",fileName);
			return false;
		}
		temp.close();
		
		if (exists == false)
			deleteFile(fileName);

		return true;

	default:
		return false;
	};

	return false;
}

bool GenericAssemblerFile::write(void* data, size_t length)
{
	if (isOpen() == false)
		return false;

	size_t len = handle.write(data,length);
	virtualAddress += len;
	return len == length;
}

bool GenericAssemblerFile::seekVirtual(u64 virtualAddress)
{
	if (virtualAddress < headerSize)
	{
		Logger::queueError(Logger::Error,L"Seeking to invalid address");
		return false;
	}

	this->virtualAddress = virtualAddress;
	u64 physicalAddress = virtualAddress-headerSize;

	if (isOpen())
		handle.setPos((long)physicalAddress);

	return true;
}

bool GenericAssemblerFile::seekPhysical(u64 physicalAddress)
{
	if ((signed)physicalAddress < 0)
	{
		Logger::queueError(Logger::Error,L"Seeking to invalid address");
		return false;
	}

	virtualAddress = physicalAddress+headerSize;

	if (isOpen())
		handle.setPos((long)physicalAddress);

	return true;
}



FileManager::FileManager()
{
	// detect own endianness
	volatile union
	{
		u32 i;
		u8 c[4];
	} u;
	u.c[3] = 0xAA;
	u.c[2] = 0xBB;
	u.c[1] = 0xCC;
	u.c[0] = 0xDD;

	if (u.i == 0xDDCCBBAA)
		ownEndianness = Endianness::Big;
	else if (u.i == 0xAABBCCDD)
		ownEndianness = Endianness::Little;
	else
		Logger::printError(Logger::Error,L"Running on unknown endianness");

	reset();
}

FileManager::~FileManager()
{

}

void FileManager::reset()
{
	activeFile = NULL;
	setEndianness(Endianness::Little);
}

bool FileManager::checkActiveFile()
{
	if (activeFile == NULL)
	{
		Logger::queueError(Logger::Error,L"No file opened");
		return false;
	}
	return true;
}

bool FileManager::openFile(AssemblerFile* file, bool onlyCheck)
{
	if (activeFile != NULL)
	{
		Logger::queueError(Logger::Warning,L"File not closed before opening a new one");
		activeFile->close();
	}

	activeFile = file;
	return activeFile->open(onlyCheck);
}

void FileManager::addFile(AssemblerFile* file)
{
	files.push_back(file);
	activeFile = file;
}

void FileManager::closeFile()
{
	if (activeFile == NULL)
	{
		Logger::queueError(Logger::Warning,L"No file opened");
		return;
	}

	activeFile->close();
	activeFile = NULL;
}

bool FileManager::write(void* data, size_t length)
{
	if (checkActiveFile() == false)
		return false;

	if (activeFile->isOpen() == false)
	{
		Logger::queueError(Logger::Error,L"No file opened");
		return false;
	}

	return activeFile->write(data,length);
}

bool FileManager::writeU8(u8 data)
{
	return write(&data,1);
}

bool FileManager::writeU16(u16 data)
{
	if (endianness != ownEndianness)
		data = swapEndianness16(data);

	return write(&data,2);
}

bool FileManager::writeU32(u32 data)
{
	if (endianness != ownEndianness)
		data = swapEndianness32(data);

	return write(&data,4);
}

u64 FileManager::getVirtualAddress()
{
	if (activeFile == NULL)
		return -1;
	return activeFile->getVirtualAddress();
}

u64 FileManager::getPhysicalAddress()
{
	if (activeFile == NULL)
		return -1;
	return activeFile->getPhysicalAddress();
}

bool FileManager::seekVirtual(u64 virtualAddress)
{
	if (checkActiveFile() == false)
		return false;

	bool result = activeFile->seekVirtual(virtualAddress);
	if (result && Global.memoryMode)
	{
		int sec = Global.symbolTable.findSection(virtualAddress);
		if (sec != -1)
			Global.Section = sec;
	}

	return result;
}

bool FileManager::seekPhysical(u64 virtualAddress)
{
	if (checkActiveFile() == false)
		return false;
	return activeFile->seekPhysical(virtualAddress);
}

bool FileManager::advanceMemory(size_t bytes)
{
	if (checkActiveFile() == false)
		return false;

	u64 pos = activeFile->getVirtualAddress();
	return activeFile->seekVirtual(pos+bytes);
}