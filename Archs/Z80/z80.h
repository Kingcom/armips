#pragma once
#include "Util/CommonClasses.h"

typedef struct {
	char* name;
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
	virtual void AssembleOpcode(char* name, char* args);
	virtual bool AssembleDirective(char* name, char* args);
	virtual void NextSection(){ };
	virtual void Pass2(){ };
	virtual void Revalidate(){ };
	virtual int GetFileAddress(int MemoryAddress){ return 0; };
	virtual int GetMemoryAddress(int FileAddress){ return 0; };
	virtual int GetWordSize() { return 2; };
};

extern CZ80Architecture z80;;

bool z80CheckImmediate(char* Source, char* Dest, int& RetLen, CStringList& List);
int z80GetRegister8(char* source, int& RetLen);
bool z80GetRegister8(char* source, int& RetLen, tZ80RegisterInfo& Result);
int z80GetRegister16(char* source, int& RetLen);
bool z80GetRegister16(char* source, int& RetLen, tZ80RegisterInfo& Result);
