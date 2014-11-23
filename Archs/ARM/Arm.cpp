#include "stdafx.h"
#include "Arm.h"
#include "Core/Common.h"
#include "Core/Directives.h"
#include "CThumbInstruction.h"
#include "CArmInstruction.h"
#include "Commands/CAssemblerLabel.h"
#include "Core/FileManager.h"
#include "ArmRelocator.h"

CArmArchitecture Arm;

bool ArmDirectiveThumb(ArgumentList& List, int flags);
bool ArmDirectiveArm(ArgumentList& List, int flags);
bool ArmDirectivePool(ArgumentList& List, int flags);
bool ArmDirectiveMsg(ArgumentList& List, int flags);
bool ArmDirectiveIdeasMsg(ArgumentList& List, int flags);

const tArmRegister ArmRegister[] = {
	{ "r0", 0, 2 },		{ "r1", 1, 2 },
	{ "r2", 2, 2 },		{ "r3", 3, 2 },
	{ "r4", 4, 2 },		{ "r5", 5, 2 },
	{ "r6", 6, 2 },		{ "r7", 7, 2 },
	{ "r8", 8, 2 },		{ "r9", 9, 2 },
	{ "r10", 10, 3 },	{ "r11", 11, 3 },
	{ "r12", 12, 3 },	{ "r13", 13, 3 },
	{ "sp", 13, 2 },	{ "r14", 14, 3 },
	{ "lr", 14, 2 },	{ "r15", 15, 3 },
	{ "pc", 15, 2 },	{ NULL, -1, 0 }
};

const tArmRegister ArmCopRegister[] = {
	{ "c0", 0, 2 },		{ "c1", 1, 2 },
	{ "c2", 2, 2 },		{ "c3", 3, 2 },
	{ "c4", 4, 2 },		{ "c5", 5, 2 },
	{ "c6", 6, 2 },		{ "c7", 7, 2 },
	{ "c8", 8, 2 },		{ "c9", 9, 2 },
	{ "c10", 10, 3 },	{ "c11", 11, 3 },
	{ "c12", 12, 3 },	{ "c13", 13, 3 },
	{ "c14", 14, 3 },	{ "c15", 15, 3 },
	{ NULL, -1, 0 }
};

const tArmRegister ArmCopNumber[] = {
	{ "p0", 0, 2 },		{ "p1", 1, 2 },
	{ "p2", 2, 2 },		{ "p3", 3, 2 },
	{ "p4", 4, 2 },		{ "p5", 5, 2 },
	{ "p6", 6, 2 },		{ "p7", 7, 2 },
	{ "p8", 8, 2 },		{ "p9", 9, 2 },
	{ "p10", 10, 3 },	{ "p11", 11, 3 },
	{ "p12", 12, 3 },	{ "p13", 13, 3 },
	{ "p14", 14, 3 },	{ "p15", 15, 3 },
	{ NULL, -1, 0 }
};

const tDirective ArmDirectives[] = {
	{ L".thumb",		0,	0,	&ArmDirectiveThumb,		0 },
	{ L".arm",			0,	0,	&ArmDirectiveArm,		0 },
	{ L".pool",			0,	0,	&ArmDirectivePool,		0 },
	{ L".msg",			1,	1,	&ArmDirectiveMsg,		0 },
	{ L".ideasmsg",		1,	2,	&ArmDirectiveIdeasMsg,	0 },
	{ NULL,				0,	0,	NULL,					0 }
};




bool CArmArchitecture::AssembleDirective(const std::wstring& name, const std::wstring& args)
{
	if (directiveAssembleGlobal(name,args) == true) return true;
	return directiveAssemble(ArmDirectives,name,args);
}

bool ArmDirectiveThumb(ArgumentList& List, int flags)
{
	Arm.SetThumbMode(true);
	ArmStateCommand* cmd = new ArmStateCommand(false);
	AddAssemblerCommand(cmd);
	return true;
}

bool ArmDirectiveArm(ArgumentList& List, int flags)
{
	Arm.SetThumbMode(false);
	ArmStateCommand* cmd = new ArmStateCommand(true);
	AddAssemblerCommand(cmd);
	return true;
}

bool ArmDirectivePool(ArgumentList& List, int flags)
{
	Arm.AssembleDirective(L".align",L"4");

	ArmPoolCommand* cmd = new ArmPoolCommand();
	AddAssemblerCommand(cmd);
	return true;
}

bool ArmDirectiveMsg(ArgumentList& List, int flags)
{
	std::wstring str = formatString(L"@@debug_msg_%08X",Global.DebugMessages++);
	std::wstring str2 = L"\"" + List[0].text + L"\"";

	Arm.AssembleOpcode(L"mov",L"r12,r12");
	Arm.AssembleOpcode(L"b",str);
	Arm.AssembleDirective(L".byte",L"0x64,0x64,0x00,0x00");
	if (List[0].text.empty()) Arm.AssembleDirective(L".byte",L"0");
	else Arm.AssembleDirective(L".ascii",str2);
	Arm.AssembleDirective(L".align",Arm.GetThumbMode() == true ? L"2" : L"4");
	addAssemblerLabel(str);
	return true;
}

bool ArmDirectiveIdeasMsg(ArgumentList& List, int flags)
{
	bool push = true;
	if (List.size() == 2)
	{
		if (List[1].text == L"false")
		{
			push = false;
		} else if (List[1].text != L"true")
		{
			Logger::printError(Logger::Error,L"Invalid arguments");
			return false;
		}
	}
	
	std::wstring label1 = formatString(L"@@debug_msg_%08X",Global.DebugMessages++);
	std::wstring label2 = formatString(L"@@debug_msg_%08X",Global.DebugMessages++);
	std::wstring str = L"\"" + List[0].text + L"\"";

	if (push == true)
	{
		if (Arm.GetThumbMode() == true)
		{
			Arm.AssembleOpcode(L"push",L"r0");
			Arm.AssembleOpcode(L"push",L"r14");
		} else {
			Arm.AssembleOpcode(L"push",L"r0,r14");
		}
	}
	
	std::wstring temp = formatString(L"r0,=%ls",label1); //-V510
	Arm.AssembleOpcode(L"add",temp);
	Arm.AssembleOpcode(L"swi",L"0xFC");

	if (push == true)
	{
		if (Arm.GetThumbMode() == true)
		{
			Arm.AssembleOpcode(L"pop",L"r0");
			Arm.AssembleOpcode(L"mov",L"r14,r0");
			Arm.AssembleOpcode(L"pop",L"r0");
		} else {
			Arm.AssembleOpcode(L"pop",L"r0,r14");
		}
	}

	Arm.AssembleOpcode(L"b",label2);
	Arm.AssembleDirective(L".align",L"4");
	addAssemblerLabel(label1);
	Arm.AssembleDirective(L".ascii",str);
	Arm.AssembleDirective(L".align",Arm.GetThumbMode() == true ? L"2" : L"4");
	addAssemblerLabel(label2);
	return true;
}


CArmArchitecture::CArmArchitecture()
{
	Pools = NULL;
	clear();
}

CArmArchitecture::~CArmArchitecture()
{
	clear();
}

void CArmArchitecture::clear()
{
	if (Pools != NULL)
		delete[] Pools;

	thumb = false;
	arm9 = false;
	PoolCount = 0;
	Pools = NULL;
}

void CArmArchitecture::Pass2()
{
	Pools = new ArmPool[PoolCount];
	CurrentPool = 0;
}

void CArmArchitecture::Revalidate()
{
	for (size_t i = 0; i < PoolCount; i++)
	{
		Pools[i].Clear();
	}
	CurrentPool = 0;
}

void CArmArchitecture::AssembleOpcode(const std::wstring& name, const std::wstring& args)
{
	if (thumb == true)	// thumb opcode
	{
		CThumbInstruction* Opcode = new CThumbInstruction();
		if (Opcode->Load((char*)convertWStringToUtf8(name).c_str(),(char*)convertWStringToUtf8(args).c_str()) == false)
		{
			delete Opcode;
			return;
		}
		AddAssemblerCommand(Opcode);
		g_fileManager->advanceMemory(Opcode->GetSize());
	} else {			// arm opcode
		CArmInstruction* Opcode = new CArmInstruction();
		if (Opcode->Load((char*)convertWStringToUtf8(name).c_str(),(char*)convertWStringToUtf8(args).c_str()) == false)
		{
			delete Opcode;
			return;
		}
		AddAssemblerCommand(Opcode);
		g_fileManager->advanceMemory(4);
	}
}

void CArmArchitecture::NextSection()
{

}

IElfRelocator* CArmArchitecture::getElfRelocator()
{
	return new ArmElfRelocator(arm9);
}

int CArmArchitecture::AddToCurrentPool(int value)
{
	if (CurrentPool == PoolCount)
	{
		return -1;
	}
	return (int) Pools[CurrentPool].AddEntry(value);
};

bool ArmGetRegister(char* source, int& RetLen, tArmRegisterInfo& Result)
{
	for (int z = 0; ArmRegister[z].name != NULL; z++)
	{
		int len = ArmRegister[z].len;
		if (strncmp(ArmRegister[z].name,source,len) == 0)	// okay so far
		{
			switch (source[len])
			{
			case '[': case ',': case ']': case '-': case '^': case '}':  case '!': case 0:
				memcpy(Result.Name,source,len);
				Result.Name[len] = 0;
				Result.Number = ArmRegister[z].num;
				RetLen = len;
				return true;
			default:
				break;
			}
		}
	}
	return false;
}

int ArmGetRegister(char* source, int& RetLen)
{
	for (int z = 0; ArmRegister[z].name != NULL; z++)
	{
		int len = ArmRegister[z].len;
		if (strncmp(ArmRegister[z].name,source,len) == 0)	// erstmal in ordnung
		{
			switch (source[len])
			{
			case '[': case ',': case ']': case '-': case '^': case '}': case 0:
				RetLen = len;
				return ArmRegister[z].num;;
			default:
				break;
			}
		}
	}
	return -1;
}

bool ArmCheckImmediate(char* Source, char* Dest, int& RetLen, CStringList& List)
{
	int BufferPos = 0;
	int l;

	if (ArmGetRegister(Source,l) != -1)	// error, quit
	{
		return false;
	}

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

		if (*Source == 0 || *Source == '\n' || *Source == ',' 
			|| *Source == ']')
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
	if (strcmp(Dest,"cpsr") == 0 || strcmp(Dest,"spsr") == 0) return false;

	if (ConvertInfixToPostfix(Dest,List) == false) return false;

	RetLen = SourceLen;
	return true;
}

bool ArmGetRlist(char* source, int& RetLen, int ValidRegisters, int& Result)
{
	int len = 0;

	int list = 0;
	while (true)
	{
		int num = ArmGetRegister(&source[len],RetLen);
		if (num == -1) return false;
		len += RetLen;

		if (source[len] == '-')
		{
			int num2 = ArmGetRegister(&source[len+1],RetLen);
			if (num2 == -1 || num2 < num) return false;
			len += (RetLen+1);
			for (int i = num; i <= num2; i++)
			{
				if ((ValidRegisters & (1 << i)) == 0) return false;
				list |= (1 << i);
			}
		} else {
			if ((ValidRegisters & (1 << num)) == 0) return false;
			list |= (1 << num);
		}
		if (source[len] != ',') break;
		len++;
	}

	RetLen = len;
	Result = list;
	return true;
}


int ArmGetShiftedImmediate(unsigned int num, int& ShiftAmount)
{
	for (int i = 0; i < 32; i+=2)
	{
		unsigned int andval = (0xFFFFFF00 >> i) | (0xFFFFFF00 << (32-i));

		if ((num & andval) == 0)	// found it
		{
			ShiftAmount = i;
			return (num << i) | (num >> (32 - i));
		}
	}
	return -1;
}

bool ArmGetCopNumber(char* source, int& RetLen, tArmRegisterInfo& Result)
{
	for (int z = 0; ArmCopNumber[z].name != NULL; z++)
	{
		int len = ArmCopNumber[z].len;
		if (strncmp(ArmCopNumber[z].name,source,len) == 0)	// okay so far
		{
			switch (source[len])
			{
			case ',': case 0:
				memcpy(Result.Name,source,len);
				Result.Name[len] = 0;
				Result.Number = ArmCopNumber[z].num;
				RetLen = len;
				return true;
			default:
				break;
			}
		}
	}
	return false;
}

bool ArmGetCopRegister(char* source, int& RetLen, tArmRegisterInfo& Result)
{
	for (int z = 0; ArmCopRegister[z].name != NULL; z++)
	{
		int len = ArmCopRegister[z].len;
		if (strncmp(ArmCopRegister[z].name,source,len) == 0)	// okay so far
		{
			switch (source[len])
			{
			case ',': case 0:
				memcpy(Result.Name,source,len);
				Result.Name[len] = 0;
				Result.Number = ArmCopRegister[z].num;
				RetLen = len;
				return true;
			default:
				break;
			}
		}
	}
	return false;
}
