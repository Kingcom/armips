#include "StdAfx.h"
#include "Util/CTable.h"
#include "Core/Common.h"
#include "Core/MathParser.h"

CTable::CTable(void)
{
	InitTable();
}

CTable::CTable(char* FileName)
{
	InitTable();
	LoadTable(FileName);
}

CTable::~CTable(void)
{
	free(Entries);
	free(Data);
}

void CTable::InitTable()
{
	Entries = (tTableEntry*) calloc(512,sizeof(tTableEntry));
	EntryCount = TABLE_NORMALSTART;
	EntriesAllocated = 512;

	Data = (unsigned char*) malloc(4*1024);
	DataPos = 0;
	DataAllocated = 4*1024;

	SetTerminator(0x00);
	open = false;
}

void CTable::ClearTable()
{
	for (int i = 0; i < 256; i++)
	{
		FirstStringEntries[i] = TABLE_TERMINATOR;
		FirstHexEntries[i] = TABLE_TERMINATOR;
	}

	DataPos = 0;
	EntryCount = TABLE_NORMALSTART;
	SetTerminator(0x00);
}

void CTable::SetTerminator(unsigned char* Hex, int HexLen)
{
	Entries[TABLE_TERMINATOR].Hex.pos = AddData(Hex,HexLen);
	Entries[TABLE_TERMINATOR].Hex.len = HexLen;
	Entries[TABLE_TERMINATOR].String.pos = 0;
	Entries[TABLE_TERMINATOR].String.len = 0;
	Entries[TABLE_TERMINATOR].NextString = TABLE_TERMINATOR;
	Entries[TABLE_TERMINATOR].NextHex = TABLE_TERMINATOR;
}

void CTable::SetTerminator(unsigned char Hex)
{
	Entries[TABLE_TERMINATOR].Hex.pos = AddData(&Hex,1);
	Entries[TABLE_TERMINATOR].Hex.len = 1;
	Entries[TABLE_TERMINATOR].String.pos = 0;
	Entries[TABLE_TERMINATOR].String.len = 0;
	Entries[TABLE_TERMINATOR].NextString = TABLE_TERMINATOR;
	Entries[TABLE_TERMINATOR].NextHex = TABLE_TERMINATOR;
}

int CTable::AddData(unsigned char* Bin, int len)
{
	if ((DataPos+len) > DataAllocated)
	{
		DataAllocated <<= 1;
		Data = (unsigned char*) realloc(Data,DataAllocated);
	}
	int pos = DataPos;
	memcpy(&Data[DataPos],Bin,len);
	DataPos += len;
	return pos;
}

void CTable::AddEntry(unsigned char Hex, char Character)
{
	AddEntry(&Hex,1,(unsigned char*)&Character,1);
}

void CTable::AddEntry(unsigned char* Hex, int HexLen, unsigned char* Text, int TextLen)
{
	// text in die liste aufnehmen
	int number = FirstStringEntries[*Text];
	if (number == TABLE_TERMINATOR)
	{
		FirstStringEntries[*Text] = EntryCount;
	} else {
		while (Entries[number].NextString != TABLE_TERMINATOR)
		{
			number = Entries[number].NextString;
		}
		Entries[number].NextString = EntryCount;
	}

	// hex in die liste aufnehmen
	number = FirstHexEntries[*Hex];
	if (number == TABLE_TERMINATOR)
	{
		FirstHexEntries[*Hex] = EntryCount;
	} else {
		while (Entries[number].NextHex != TABLE_TERMINATOR)
		{
			number = Entries[number].NextHex;
		}
		Entries[number].NextHex = EntryCount;
	}

	// prüfen ob noch platz da ist
	if (EntryCount == EntriesAllocated)
	{
		EntriesAllocated <<= 1;
		Entries = (tTableEntry*) realloc(Entries,EntriesAllocated*sizeof(tTableEntry));
	}

	// und jetzt den eintrag füllen
	Entries[EntryCount].Hex.pos = AddData(Hex,HexLen);
	Entries[EntryCount].Hex.len = HexLen;
	Entries[EntryCount].String.pos = AddData((unsigned char*)Text,TextLen);
	Entries[EntryCount].NextString = TABLE_TERMINATOR;
	Entries[EntryCount++].String.len = TextLen;
}

bool CTable::LoadTable(char* FileName)
{
	char str[255];

	ClearTable();
	FILE* Input = fopen(FileName,"r");
	if (CheckBom(Input) == false) return false;

	unsigned char HexString[32];
	char TextString[64];

	while (!feof(Input))
	{
		fgets(str,255,Input);
		if (feof(Input)) break;
		if (strcmp(str,"\n") == 0) continue;
		int pos = 0;

		int HexLen = 0;
		if (str[0] == '/')
		{
			pos++;
			while (str[pos] != '\n' && str[pos] != 0)
			{
				HexString[HexLen++] = HexToInt(&str[pos],2);
				pos += 2;
			}
			SetTerminator(HexString,HexLen);
		} else if (str[0] != '*')
		{
			if (strchr(str,'=') == NULL) return false;
			while (str[pos] != '=')
			{
				HexString[HexLen++] = HexToInt(&str[pos],2);
				pos += 2;
			}
			pos++;
			int TextLen = 0;
			while (str[pos] != '\n' && str[pos] != 0)
			{
				TextString[TextLen++] = str[pos++];
			}

			AddEntry(HexString,HexLen,(unsigned char*)TextString,TextLen);
		}
	}
	fclose(Input);
	open = true;
	return true;
}

int CTable::SearchHexMatch(unsigned char* Data)
{
	int longestmatch = 0;
	int longestnum = -1;
	int Number = FirstHexEntries[*Data];

	while (Number != TABLE_TERMINATOR)
	{
		int TempLen = GetHexLen(Number);
		if (TempLen > longestmatch)
		{
			if (memcmp(Data,GetHexPointer(Number),TempLen) == 0)
			{
				longestmatch = TempLen;
				longestnum = Number;
			}
		}
		Number = Entries[Number].NextHex;
	}

	return longestnum;
}


int CTable::SearchStringMatch(unsigned char* String)
{
	int longestmatch = 0;
	int longestnum = -1;
	int Number = FirstStringEntries[*String];

	while (Number != TABLE_TERMINATOR)
	{
		int TempLen = GetStringLen(Number);
		if (TempLen > longestmatch)
		{
			if (memcmp(String,GetStringPointer(Number),TempLen) == 0)
			{
				longestmatch = TempLen;
				longestnum = Number;
			}
		}
		Number = Entries[Number].NextString;
	}

	return longestnum;
}

int CTable::EncodeString(unsigned char* String, unsigned char* Output)
{
	int OutputPos = 0;
	int l;

	while (*String != 0)
	{
		if ((l = SearchStringMatch(String)) == -1) return -1;

		String += GetStringLen(l);
		memcpy(&Output[OutputPos],GetHexPointer(l),GetHexLen(l));
		OutputPos += GetHexLen(l);
	}

	// terminieren
	memcpy(&Output[OutputPos],GetHexPointer(TABLE_TERMINATOR),GetHexLen(TABLE_TERMINATOR));
	OutputPos += GetHexLen(TABLE_TERMINATOR);

	return OutputPos;
}

int CTable::WriteTerminator(unsigned char* Output)
{
	memcpy(Output,GetHexPointer(TABLE_TERMINATOR),GetHexLen(TABLE_TERMINATOR));
	return GetHexLen(TABLE_TERMINATOR);
}