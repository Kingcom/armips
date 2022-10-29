#include "FileManager.h"

#include "Core/Common.h"
#include "Core/Misc.h"
#include "Util/FileSystem.h"
#include "Util/Util.h"

inline uint64_t swapEndianness64(uint64_t value)
{
	return ((value & 0xFF) << 56) | ((value & 0xFF00) << 40) | ((value & 0xFF0000) << 24) | ((value & 0xFF000000) << 8) |
	((value & 0xFF00000000) >> 8) | ((value & 0xFF0000000000) >> 24) |
	((value & 0xFF000000000000) >> 40) | ((value & 0xFF00000000000000) >> 56);
}

inline uint32_t swapEndianness32(uint32_t value)
{
	return ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) | ((value & 0xFF0000) >> 8) | ((value & 0xFF000000) >> 24);
}

inline uint16_t swapEndianness16(uint16_t value)
{
	return ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);
}


GenericAssemblerFile::GenericAssemblerFile(const fs::path& fileName, int64_t headerSize, bool overwrite)
	: originalHeaderSize(headerSize), headerSize(headerSize), fileName(fileName)
{
	this->seekPhysical(0);
	mode = overwrite ? Create : Open;
}

GenericAssemblerFile::GenericAssemblerFile(const fs::path& fileName, const fs::path& originalFileName, int64_t headerSize)
	: originalHeaderSize(headerSize), headerSize(headerSize), fileName(fileName), originalName(originalFileName)
{
	this->seekPhysical(0);
	mode = Copy;
}

bool GenericAssemblerFile::open(bool onlyCheck)
{
	std::error_code errorCode;

	headerSize = originalHeaderSize;
	virtualAddress = headerSize;

	auto flagsOpenExisting = fs::ofstream::in | fs::ofstream::out | fs::ofstream::binary;
	auto flagsOverwrite = fs::ofstream::out | fs::ofstream::trunc | fs::ofstream::binary;

	if (!onlyCheck)
	{
		// actually open the file
		switch (mode)
		{
		case Open:
			stream.open(fileName, flagsOpenExisting);
			if (!stream.is_open())
			{
				Logger::printError(Logger::FatalError, "Could not open file %s",fileName.u8string());
				return false;
			}
			return true;

		case Create:
			stream.open(fileName, flagsOverwrite);
			if (!stream.is_open())
			{
				Logger::printError(Logger::FatalError, "Could not create file %s",fileName.u8string());
				return false;
			}
			return true;

		case Copy:
			if (!fs::copy_file(originalName, fileName, fs::copy_options::overwrite_existing, errorCode))
			{
				Logger::printError(Logger::FatalError, "Could not copy file %s",originalName.u8string());
				return false;
			}

			stream.open(fileName, flagsOpenExisting);
			if (!stream.is_open())
			{
				Logger::printError(Logger::FatalError, "Could not create file %s",fileName.u8string());
				return false;
			}
			return true;
		}
	}

	// else only check if it can be done, don't actually do it permanently
	bool exists = false;
	fs::ofstream temp;
	switch (mode)
	{
	case Open:
		temp.open(fileName, flagsOpenExisting);
		if (!temp.is_open())
		{
			Logger::queueError(Logger::FatalError, "Could not open file %s",fileName.u8string());
			return false;
		}
		temp.close();
		return true;

	case Create:
		// open file with writee access. if it didn't exist before, remove it afterwards
		exists = fs::exists(fileName);

		temp.open(fileName, exists ? flagsOpenExisting : flagsOverwrite);
		if (!temp.is_open())
		{
			Logger::queueError(Logger::FatalError, "Could not create file %s",fileName.u8string());
			return false;
		}
		temp.close();

		if (!exists)
			fs::remove(fileName, errorCode);

		return true;

	case Copy:
		// check original file
		temp.open(originalName, flagsOpenExisting);
		if (!temp.is_open())
		{
			Logger::queueError(Logger::FatalError, "Could not copy file %s",originalName.u8string());
			return false;
		}
		temp.close();

		// Check input and output are not the same
		std::error_code ec;
		if (fs::equivalent(originalName, fileName, ec))
		{
			Logger::queueError(Logger::FatalError, "Could not copy file %s", originalName.u8string());
			return false;
		}

		// check new file, same as create
		exists = fs::exists(fileName);

		temp.open(fileName, exists ? flagsOpenExisting : flagsOverwrite);
		if (!temp.is_open())
		{
			Logger::queueError(Logger::FatalError, "Could not create file %s",fileName.u8string());
			return false;
		}
		temp.close();

		if (!exists)
			fs::remove(fileName, errorCode);
		return true;
	}

	return false;
}

bool GenericAssemblerFile::write(void* data, size_t length)
{
	if (!isOpen())
		return false;

	stream.write(reinterpret_cast<const char *>( data ), length);
	virtualAddress += length;
	return !stream.fail();
}

bool GenericAssemblerFile::seekVirtual(int64_t virtualAddress)
{
	if (virtualAddress - headerSize < 0)
	{
		Logger::queueError(Logger::Error, "Seeking to virtual address with negative physical address");
		return false;
	}
	if (virtualAddress < 0)
		Logger::queueError(Logger::Warning, "Seeking to negative virtual address");

	this->virtualAddress = virtualAddress;
	int64_t physicalAddress = virtualAddress-headerSize;

	if (isOpen())
		stream.seekp(physicalAddress);

	return true;
}

bool GenericAssemblerFile::seekPhysical(int64_t physicalAddress)
{
	if (physicalAddress < 0)
	{
		Logger::queueError(Logger::Error, "Seeking to negative physical address");
		return false;
	}
	if (physicalAddress + headerSize < 0)
		Logger::queueError(Logger::Warning, "Seeking to physical address with negative virtual address");

	virtualAddress = physicalAddress+headerSize;

	if (isOpen())
		stream.seekp(physicalAddress);

	return true;
}



FileManager::FileManager()
{
	// detect own endianness
	volatile union
	{
		uint32_t i;
		uint8_t c[4];
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
		Logger::printError(Logger::Error, "Running on unknown endianness");

	reset();
}

FileManager::~FileManager() = default;

void FileManager::reset()
{
	activeFile = nullptr;
	setEndianness(Endianness::Little);
}

bool FileManager::checkActiveFile()
{
	if (activeFile == nullptr)
	{
		Logger::queueError(Logger::Error, "No file opened");
		return false;
	}
	return true;
}

bool FileManager::openFile(std::shared_ptr<AssemblerFile> file, bool onlyCheck)
{
	if (activeFile != nullptr)
	{
		Logger::queueError(Logger::Warning, "File not closed before opening a new one");
		activeFile->close();
	}

	activeFile = std::move(file);
	return activeFile->open(onlyCheck);
}

void FileManager::addFile(std::shared_ptr<AssemblerFile> file)
{
	files.push_back(file);
}

void FileManager::closeFile()
{
	if (activeFile == nullptr)
	{
		Logger::queueError(Logger::Warning, "No file opened");
		return;
	}

	activeFile->close();
	activeFile = nullptr;
}

bool FileManager::write(void* data, size_t length)
{
	if (!checkActiveFile())
		return false;

	if (!activeFile->isOpen())
	{
		Logger::queueError(Logger::Error, "No file opened");
		return false;
	}

	return activeFile->write(data,length);
}

bool FileManager::writeU8(uint8_t data)
{
	return write(&data,1);
}

bool FileManager::writeU16(uint16_t data)
{
	if (endianness != ownEndianness)
		data = swapEndianness16(data);

	return write(&data,2);
}

bool FileManager::writeU32(uint32_t data)
{
	if (endianness != ownEndianness)
		data = swapEndianness32(data);

	return write(&data,4);
}

bool FileManager::writeU64(uint64_t data)
{
	if (endianness != ownEndianness)
		data = swapEndianness64(data);

	return write(&data,8);
}

int64_t FileManager::getVirtualAddress()
{
	if (activeFile == nullptr)
		return -1;
	return activeFile->getVirtualAddress();
}

int64_t FileManager::getPhysicalAddress()
{
	if (activeFile == nullptr)
		return -1;
	return activeFile->getPhysicalAddress();
}

int64_t FileManager::getHeaderSize()
{
	if (activeFile == nullptr)
		return -1;
	return activeFile->getHeaderSize();
}

bool FileManager::seekVirtual(int64_t virtualAddress)
{
	if (!checkActiveFile())
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

bool FileManager::seekPhysical(int64_t virtualAddress)
{
	if (!checkActiveFile())
		return false;
	return activeFile->seekPhysical(virtualAddress);
}

bool FileManager::advanceMemory(size_t bytes)
{
	if (!checkActiveFile())
		return false;

	int64_t pos = activeFile->getVirtualAddress();
	return activeFile->seekVirtual(pos+bytes);
}

int64_t FileManager::getOpenFileID()
{
	if (!checkActiveFile())
		return 0;

	static_assert(sizeof(int64_t) >= sizeof(intptr_t), "Assumes pointers are <= 64 bit");
	return (int64_t)(intptr_t)activeFile.get();
}
