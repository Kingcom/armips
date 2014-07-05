#pragma once
#include "Archs/Architecture.h"
#include "Util/CommonClasses.h"

typedef struct {
	const char* name;
	short num;
	short len;
} tZ80Register;

typedef struct {
	char Name[5];
	short Number;
} tZ80RegisterInfo;

extern const tZ80Register Z80Registers8[];
extern const tZ80Register Z80Registers16[];

class CZ80Architecture: public CArchitecture
{
public:
	virtual void AssembleOpcode(const std::wstring& name, const std::wstring& args);
	virtual bool AssembleDirective(const std::wstring& name, const std::wstring& args);
	virtual void NextSection(){ };
	virtual void Pass2(){ };
	virtual void Revalidate(){ };
	virtual int GetWordSize() { return 2; };
	virtual IElfRelocator* getElfRelocator() { return NULL; };
	virtual Endianness getEndianness() { return Endianness::Little; };
};

extern CZ80Architecture z80;;

bool z80CheckImmediate(char* Source, char* Dest, int& RetLen, CStringList& List);
int z80GetRegister8(char* source, int& RetLen);
bool z80GetRegister8(char* source, int& RetLen, tZ80RegisterInfo& Result);
int z80GetRegister16(char* source, int& RetLen);
bool z80GetRegister16(char* source, int& RetLen, tZ80RegisterInfo& Result);
