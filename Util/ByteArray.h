#pragma once

typedef unsigned char byte;
class ByteArray
{
public:
	ByteArray();
	ByteArray(const ByteArray& other);
	ByteArray(byte* data, int size);
	ByteArray(ByteArray&& other);
	~ByteArray();
	ByteArray& operator=(ByteArray& other);
	ByteArray& operator=(ByteArray&& other);

	int append(const ByteArray& other);
	int append(byte* data, int size);
	int appendByte(byte b) { return append(&b,1); };
	void replaceByte(int pos, byte b) { data_[pos] = b; };
	void replaceBytes(int pos, byte* data, int size);
	void reserveBytes(int count, byte value = 0);
	void alignSize(int alignment);

	int getDoubleWord(int pos, bool bigEndian = false)
	{
		if (pos+3 >= this->size() || pos < 0) return -1;
		unsigned char* d = (unsigned char*) this->data();

		if (bigEndian == false)
		{
			return d[pos+0] | (d[pos+1] << 8) | (d[pos+2] << 16) | (d[pos+3] << 24);
		} else {
			return d[pos+3] | (d[pos+2] << 8) | (d[pos+1] << 16) | (d[pos+0] << 24);
		}
	};
	
	void replaceDoubleWord(int pos, unsigned int w, bool bigEndian = false)
	{
		if (pos+3 >= this->size() || pos < 0) return;
		unsigned char* d = (unsigned char*) this->data();
		
		if (bigEndian == false)
		{
			d[pos+0] = w & 0xFF;
			d[pos+1] = (w >> 8) & 0xFF;
			d[pos+2] = (w >> 16) & 0xFF;
			d[pos+3] = (w >> 24) & 0xFF;
		} else {
			d[pos+0] = (w >> 24) & 0xFF;
			d[pos+1] = (w >> 16) & 0xFF;
			d[pos+2] = (w >> 8) & 0xFF;
			d[pos+3] = w & 0xFF;
		}
	}

	byte& operator [](unsigned int index)
	{
		return data_[index];
	};
	
	const byte& operator [](unsigned int index) const
	{
		return data_[index];
	};

	int size() const { return size_; };
	byte* data(int pos = 0) const { return &data_[pos]; };
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
