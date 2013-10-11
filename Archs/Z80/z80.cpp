#include "stdafx.h"
#include "Core/Common.h"
#include "z80.h"
#include "Core/MathParser.h"
#include "CZ80Instruction.h"
#include "Core/Directives.h"
#include "Core/FileManager.h"

const tZ80Register Z80Registers8[] = {
	{ "b",		0,		1 },
	{ "c",		1,		1 },
	{ "d",		2,		1 },
	{ "e",		3,		1 },
	{ "h",		4,		1 },
	{ "l",		5,		1 },
	{ "(hl)",	6,		4 },
	{ "a",		7,		1 },
	{ NULL,		0xFF,	0xFF }
};

const tZ80Register Z80Registers16[] = {
	{ "bc",		0,		2 },
	{ "de",		1,		2 },
	{ "hl",		2,		2 },
	{ "sp",		3,		2 },
	{ NULL,		0xFF,	0xFF }
};

CZ80Architecture z80;


bool CZ80Architecture::AssembleDirective(const std::wstring& name, const std::wstring& args)
{
	if (directiveAssembleGlobal(name,args) == true) return true;
	return false;
}

void CZ80Architecture::AssembleOpcode(const std::wstring& name, const std::wstring& args)
{
	CZ80Instruction* Opcode = new CZ80Instruction();
	if (Opcode->Load((char*)convertWStringToUtf8(name).c_str(),(char*)convertWStringToUtf8(args).c_str()) == false)
	{
		delete Opcode;
		return;
	}
	AddAssemblerCommand(Opcode);
	g_fileManager->advanceMemory(Opcode->GetSize());
}

bool z80CheckImmediate(char* Source, char* Dest, int& RetLen, CStringList& List)
{
	int BufferPos = 0;
	int l;

	if (z80GetRegister8(Source,l) != -1) return false;
	if (z80GetRegister16(Source,l) != -1) return false;

	int SourceLen = 0;

	while (true)
	{
		if (*Source == '\'' && *(Source+2) == '\'')
		{
			Dest[BufferPos++] = *Source++;
			Dest[BufferPos++] = *Source++;
			Dest[BufferPos++] = *Source++;
			SourceLen += 3;
			continue;
		}

		if (*Source == 0 || *Source == '\n' || *Source == ',' )
		{
			Dest[BufferPos] = 0;
			break;
		}

		if (*Source == ' ' || *Source == '\t')
		{
			Source++;
			SourceLen++;
			continue;
		}

		Dest[BufferPos++] = *Source++;
		SourceLen++;
	}

	if (BufferPos == 0) return false;

	if (ConvertInfixToPostfix(Dest,List) == false) return false;

	RetLen = SourceLen;
	return true;
}

int z80GetRegister8(char* source, int& RetLen)
{
	for (int z = 0; Z80Registers8[z].name != NULL; z++)
	{
		int len = Z80Registers8[z].len;
		if (strncmp(Z80Registers8[z].name,source,len) == 0)
		{
			switch (source[len])
			{
			case ')': case ',': case 0:
				RetLen = len;
				return Z80Registers8[z].num;
			default:
				break;
			}
		}
	}
	return -1;
}

bool z80GetRegister8(char* source, int& RetLen, tZ80RegisterInfo& Result)
{
	for (int z = 0; Z80Registers8[z].name != NULL; z++)
	{
		int len = Z80Registers8[z].len;
		if (strncmp(Z80Registers8[z].name,source,len) == 0)
		{
			switch (source[len])
			{
			case ')': case ',': case 0:
				memcpy(Result.Name,source,len);
				Result.Name[len] = 0;
				Result.Number = Z80Registers8[z].num;
				RetLen = len;
				return true;
			default:
				break;
			}
		}
	}
	return false;
}

int z80GetRegister16(char* source, int& RetLen)
{
	for (int z = 0; Z80Registers16[z].name != NULL; z++)
	{
		int len = Z80Registers16[z].len;
		if (strncmp(Z80Registers16[z].name,source,len) == 0)
		{
			switch (source[len])
			{
			case ')': case ',': case 0:
				RetLen = len;
				return Z80Registers16[z].num;
			default:
				break;
			}
		}
	}
	return -1;
}

bool z80GetRegister16(char* source, int& RetLen, tZ80RegisterInfo& Result)
{
	for (int z = 0; Z80Registers16[z].name != NULL; z++)
	{
		int len = Z80Registers16[z].len;
		if (strncmp(Z80Registers16[z].name,source,len) == 0)
		{
			switch (source[len])
			{
			case ')': case ',': case 0:
				memcpy(Result.Name,source,len);
				Result.Name[len] = 0;
				Result.Number = Z80Registers16[z].num;
				RetLen = len;
				return true;
			default:
				break;
			}
		}
	}
	return false;
}
