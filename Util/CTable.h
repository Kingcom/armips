#pragma once

#define TABLE_TERMINATOR	0
#define TABLE_NORMALSTART	1

typedef struct
{
	int pos;
	int len;
} tTablePointer;

typedef struct
{
	tTablePointer String;
	tTablePointer Hex;
	unsigned short NextString;
	unsigned short NextHex;
} tTableEntry;


class CTable
{
public:
	CTable(void);
	CTable(char* FileName);
	~CTable(void);
	bool LoadTable(char* FileName);
	int EncodeString(unsigned char* String, unsigned char* Output);
	bool IsOpen() { return open; };
	void SetTerminator(unsigned char* Hex, int HexLen);
	void SetTerminator(unsigned char Hex);
	void AddEntry(unsigned char Hex, char Character);
	void AddEntry(unsigned char* Hex, int HexLen, unsigned char* Text, int TextLen);
	int SearchHexMatch(unsigned char* Data);
	int SearchStringMatch(unsigned char* String);
	const unsigned char* GetHexPointer(int num) { return &Data[Entries[num].Hex.pos]; };
	const char* GetStringPointer(int num) { return (char*)&Data[Entries[num].String.pos]; };
	int GetHexLen(int num) { return Entries[num].Hex.len; };
	int GetStringLen(int num) { return Entries[num].String.len; };
	int WriteTerminator(unsigned char* Output);
private:
	void InitTable();
	void ClearTable();
	int AddData(unsigned char* Bin, int len);
	tTableEntry* Entries;
	int EntryCount;
	int EntriesAllocated;
	unsigned char* Data;
	int DataPos;
	int DataAllocated;
	bool open;
	unsigned short FirstStringEntries[256];	// der erste eintrag für jeden anfangsbuchstaben
	unsigned short FirstHexEntries[256];	// der erste eintrag für jedes byte
};
