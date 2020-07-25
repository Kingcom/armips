#include "Util/ByteArray.h"

#include "Util/Util.h"

#include <cstring>

ByteArray::ByteArray()
{
	data_ = nullptr;
	size_ = allocatedSize_ = 0;
}

ByteArray::ByteArray(const ByteArray& other)
{
	data_ = nullptr;
	size_ = allocatedSize_ = 0;
	append(other);
}

ByteArray::ByteArray(byte* data, size_t size)
{
	data_ = nullptr;
	size_ = allocatedSize_ = 0;
	append(data,size);
}

ByteArray::ByteArray(ByteArray&& other)
{
	data_ = other.data_;
	size_ = other.size_;
	allocatedSize_ = other.allocatedSize_;
	other.data_ = nullptr;
	other.allocatedSize_ = other.size_ = 0;
}

ByteArray::~ByteArray()
{
	free(data_);
}

ByteArray& ByteArray::operator=(ByteArray& other)
{
	free(data_);
	data_ = nullptr;
	size_ = allocatedSize_ = 0;
	append(other);

	return *this;
}

ByteArray& ByteArray::operator=(ByteArray&& other)
{
	data_ = other.data_;
	size_ = other.size_;
	allocatedSize_ = other.allocatedSize_;
	other.data_ = nullptr;
	other.allocatedSize_ = other.size_ = 0;
	return *this;
}

void ByteArray::grow(size_t neededSize)
{
	if (neededSize < allocatedSize_) return;

	// align to next 0.5kb... it's a start
	allocatedSize_ = ((neededSize+511)/512)*512;
	if (data_ == nullptr)
	{
		data_ = (byte*) malloc(allocatedSize_);
	} else {
		data_ = (byte*) realloc(data_,allocatedSize_);
	}
}

size_t ByteArray::append(const ByteArray& other)
{
	size_t oldSize = size();
	size_t otherSize = other.size();
	grow(size()+otherSize);
	memcpy(&data_[size_],other.data(),otherSize);
	size_ += otherSize;
	return oldSize;
}

size_t ByteArray::append(void* data, size_t size)
{
	size_t oldSize = this->size();
	grow(this->size()+size);
	memcpy(&data_[size_],data,size);
	this->size_ += size;
	return oldSize;
}

void ByteArray::replaceBytes(size_t pos, byte* data, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		replaceByte(pos+i,data[i]);
	}
}

void ByteArray::reserveBytes(size_t count, byte value)
{
	grow(this->size()+count);
	memset(&data_[size_],value,count);
	size_ += count;
}

void ByteArray::alignSize(size_t alignment)
{
	if (alignment <= 0) return;

	while (size_ % alignment)
	{
		appendByte(0);
	}
}

void ByteArray::resize(size_t newSize)
{
	grow(newSize);
	size_ = newSize;
}

ByteArray ByteArray::mid(size_t start, ssize_t length)
{
	ByteArray ret;

	if (length < 0)
		length = size_-start;

	if (start >= size_)
		return ret;

	ret.grow(length);
	ret.size_ = length;
	memcpy(ret.data_,&data_[start],length);
	return ret;
}

ByteArray ByteArray::fromFile(const fs::path& fileName, long start, size_t size)
{
	fs::ifstream stream(fileName, fs::fstream::in | fs::fstream::binary);
	if (!stream.is_open())
		return {};

	auto fileSize = fs::file_size(fileName);
	if (start >= fileSize)
		return {};

	if (size == 0 || start+(long)size > fileSize)
		size = fileSize-start;

	stream.seekg(start);

	ByteArray ret;
	ret.grow(size);

	stream.read(reinterpret_cast<char *>(ret.data()), size);
	ret.size_ = stream.gcount();

	return ret;
}

bool ByteArray::toFile(const fs::path& fileName)
{
	fs::ofstream stream(fileName, fs::fstream::out | fs::fstream::binary | fs::fstream::trunc);
	if (!stream.is_open())
		return {};

	stream.write(reinterpret_cast<const char *>(data_), size_);
	return !stream.fail();
}
