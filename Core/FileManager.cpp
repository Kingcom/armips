#include "stdafx.h"
#include "FileManager.h"
#include "Misc.h"

bool copyFile(const std::wstring& existingFile, const std::wstring& newFile)
{
#ifdef _WIN32
	return CopyFileW(existingFile.c_str(),newFile.c_str(),false) != FALSE;
#else
	// good question...
	return false;
#endif
}

bool deleteFile(const std::wstring& fileName)
{
#ifdef _WIN32
	return DeleteFileW(fileName.c_str()) != FALSE;
#else
	// good question...
	return false;
#endif
}


GenericAssemblerFile::GenericAssemblerFile(const std::wstring& fileName, int headerSize, bool overwrite)
{
	this->fileName = fileName;
	this->originalHeaderSize = headerSize;
	mode = overwrite == true ? Create : Open;
}

GenericAssemblerFile::GenericAssemblerFile(const std::wstring& fileName, const std::wstring& originalFileName, int headerSize)
{
	this->fileName = fileName;
	this->originalName = originalFileName;
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
				Logger::printError(Logger::FatalError,L"Could not open file %s",fileName.c_str());
				return false;
			}
			return true;

		case Create:
			success = handle.open(fileName,BinaryFile::Write);
			if (success == false)
			{
				Logger::printError(Logger::FatalError,L"Could not create file %s",fileName.c_str());
				return false;
			}
			return true;

		case Copy:
			success = copyFile(originalName,fileName);
			if (success == false)
			{
				Logger::printError(Logger::FatalError,L"Could not copy file %s",originalName.c_str());
				return false;
			}

			success = handle.open(fileName,BinaryFile::ReadWrite);
			if (success == false)
			{
				Logger::printError(Logger::FatalError,L"Could not open file %s",fileName.c_str());
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
			Logger::queueError(Logger::FatalError,L"Could not open file %s",fileName.c_str());
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
			Logger::queueError(Logger::FatalError,L"Could not create file %s",fileName.c_str());
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
			Logger::queueError(Logger::FatalError,L"Could not open file %s",originalName.c_str());
			return false;
		}
		temp.close();

		// check new file, same as create
		exists = fileExists(fileName);
		success = temp.open(fileName,exists ? BinaryFile::ReadWrite : BinaryFile::Write);
		if (success == false)
		{
			Logger::queueError(Logger::FatalError,L"Could not open file %s",fileName.c_str());
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

bool GenericAssemblerFile::write(void* data, int length)
{
	if (isOpen() == false)
		return false;

	int len = handle.write(data,length);
	virtualAddress += len;
	return len == length;
}

bool GenericAssemblerFile::seekVirtual(size_t virtualAddress)
{
	this->virtualAddress = virtualAddress;
	size_t physicalAddress = virtualAddress-headerSize;

	if (isOpen())
		handle.setPos(physicalAddress);

	return true;
}

bool GenericAssemblerFile::seekPhysical(size_t physicalAddress)
{
	virtualAddress = physicalAddress+headerSize;

	if (isOpen())
		handle.setPos(physicalAddress);

	return true;
}



FileManager::FileManager()
{
	reset();
}

FileManager::~FileManager()
{

}

void FileManager::reset()
{
	activeFile = NULL;
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

bool FileManager::write(void* data, int length)
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

size_t FileManager::getVirtualAddress()
{
	if (checkActiveFile() == false)
		return -1;
	return activeFile->getVirtualAddress();
}

size_t FileManager::getPhysicalAddress()
{
	if (checkActiveFile() == false)
		return -1;
	return activeFile->getPhysicalAddress();
}

bool FileManager::seekVirtual(size_t virtualAddress)
{
	if (checkActiveFile() == false)
		return false;
	return activeFile->seekVirtual(virtualAddress);
}

bool FileManager::seekPhysical(size_t virtualAddress)
{
	if (checkActiveFile() == false)
		return false;
	return activeFile->seekPhysical(virtualAddress);
}

bool FileManager::advanceMemory(int bytes)
{
	if (checkActiveFile() == false)
		return false;

	int pos = activeFile->getVirtualAddress();
	return activeFile->seekVirtual(pos+bytes);
}