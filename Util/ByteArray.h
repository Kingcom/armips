#pragma once

typedef unsigned char byte;
class ByteArray
{
public:
	ByteArray();
	ByteArray(ByteArray& other);
	ByteArray(byte* data, int size);
	ByteArray(ByteArray&& other);
	~ByteArray();
	ByteArray& operator=(ByteArray& other);
	ByteArray& operator=(ByteArray&& other);

	int append(ByteArray& other);
	int append(byte* data, int size);
	int appendByte(byte b) { return append(&b,1); };
	void replaceByte(int pos, byte b) { data_[pos] = b; };
	void replaceBytes(int pos, byte* data, int size);
	void reserveBytes(int count, byte value = 0);
	void alignSize(int alignment);

	byte& operator [](unsigned int index)
	{
		return data_[index];
	};
	
	const byte& operator [](unsigned int index) const
	{
		return data_[index];
	};

	int size() { return size_; };
	byte* data(int pos = 0) { return &data_[pos]; };
	void clear() { size_ = 0; };
	void resize(int newSize);
	ByteArray mid(int start, int length = 0);
	ByteArray left(int length) { return mid(0,length); };
	ByteArray right(int length) { return mid(size_-length,length); };

	static ByteArray fromFile(const std::wstring& fileName, int start = 0, int size = -1);
	bool toFile(const std::wstring& fileName);
private:
	void grow(int neededSize);
	byte* data_;
	int size_;
	int allocatedSize_;
};
