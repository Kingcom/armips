#include "stdafx.h"
#include "ByteArray.h"
#include <windows.h>

ByteArray::ByteArray()
{
	data_ = NULL;
	size_ = allocatedSize_ = 0;
}

ByteArray::ByteArray(ByteArray& other)
{
	data_ = NULL;
	size_ = allocatedSize_ = 0;
	append(other);
}

ByteArray::ByteArray(byte* data, int size)
{
	data_ = NULL;
	size_ = allocatedSize_ = 0;
	append(data,size);
}

ByteArray::ByteArray(ByteArray&& other)
{
	data_ = other.data_;
	size_ = other.size_;
	allocatedSize_ = other.allocatedSize_;
	other.data_ = NULL;
	other.allocatedSize_ = other.size_ = 0;
}

ByteArray::~ByteArray()
{
	free(data_);
}

ByteArray& ByteArray::operator=(ByteArray& other)
{
	free(data_);
	data_ = NULL;
	size_ = allocatedSize_ = 0;
	append(other);

	return *this;
}

ByteArray& ByteArray::operator=(ByteArray&& other)
{
	data_ = other.data_;
	size_ = other.size_;
	allocatedSize_ = other.allocatedSize_;
	other.data_ = NULL;
	other.allocatedSize_ = other.size_ = 0;
	return *this;
}

void ByteArray::grow(int neededSize)
{
	if (neededSize < allocatedSize_) return;

	// align to next 0.5kb... it's a start
	allocatedSize_ = ((neededSize+511)/512)*512;
	if (data_ == NULL)
	{
		data_ = (byte*) malloc(allocatedSize_);
	} else {
		data_ = (byte*) realloc(data_,allocatedSize_);
	}
}

int ByteArray::append(ByteArray& other)
{
	int oldSize = size();
	int otherSize = other.size();
	grow(size()+otherSize);
	memcpy(&data_[size_],other.data(),otherSize);
	size_ += otherSize;
	return oldSize;
}

int ByteArray::append(byte* data, int size)
{
	int oldSize = this->size();
	grow(this->size()+size);
	memcpy(&data_[size_],data,size);
	this->size_ += size;
	return oldSize;
}

void ByteArray::replaceBytes(int pos, byte* data, int size)
{
	for (int i = 0; i < size; i++)
	{
		replaceByte(pos+i,data[i]);
	}
}

void ByteArray::reserveBytes(int count, byte value)
{
	grow(this->size()+count);
	for (int i = 0; i < count; i++)
	{
		data_[size_++] = value;
	}
}

void ByteArray::alignSize(int alignment)
{
	if (alignment <= 0) return;

	while (size_ % alignment)
	{
		appendByte(0);
	}
}

void ByteArray::resize(int newSize)
{
	grow(newSize);
	size_ = newSize;
}

ByteArray ByteArray::mid(int start, int length)
{
	ByteArray ret;

	if (length <= 0)
		length = size_-start;

	if (start >= size_)
		return ret;

	ret.grow(length);
	ret.size_ = length;
	memcpy(ret.data_,&data_[start],length);
	return ret;
}

ByteArray ByteArray::fromFile(const std::wstring& fileName, int start, int size)
{
	ByteArray ret;
	
	FILE* input = _wfopen(fileName.c_str(),L"rb");
	if (input == NULL)
		return ret;

	fseek(input,0,SEEK_END);
	int fileSize = ftell(input);

	if (start >= fileSize)
	{
		fclose(input);
		return ret;
	}

	if (size == -1 || start+size > fileSize)
		size = fileSize-start;

	fseek(input,start,SEEK_SET);

	ret.grow(size);
	ret.size_ = size;

	fread(ret.data(),1,size,input);
	fclose(input);

	return ret;
}

bool ByteArray::toFile(const std::wstring& fileName)
{
	FILE* output = _wfopen(fileName.c_str(),L"wb");
	if (output == NULL) return false;
	int length = fwrite(data_,1,size_,output);
	fclose(output);
	return length == size_;
}