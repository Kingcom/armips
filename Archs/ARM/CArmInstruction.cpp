#include "stdafx.h"
#include "CArmInstruction.h"
#include "Core/Common.h"
#include "Arm.h"
#include "Core/FileManager.h"
#include <cstddef>

const char ArmConditions[16][3] = {
	"eq","ne","cs","cc","mi","pl","vs","vc","hi","ls","ge","lt","gt","le","","nv"
};
const char ArmAddressingModes[4][3] = { "da","ia","db","ib"};
const char ArmShiftModes[4][4] = { "lsl", "lsr", "asr", "ror" };

const char ArmPsrModes[16][5] = {
	"_???",	"_ctl",	"_x",	"_xc",	"_s",	"_sc",	"_sx",	"_sxc",
	"_flg",	"_fc",	"_fx",	"_fxc",	"_fs",	"_fsc",	"_fsx",	""
};


CArmInstruction::CArmInstruction()
{
	NoCheckError = false;
	Loaded = false;
	Vars.Shift.UseShift = false;
	Vars.Shift.UseFinal = false;
}

int ArmGetCondition(char*& string)
{
	char c1 = string[0];
	char c2 = string[1];
	string += 2;

	if (c1 == 'e' && c2 == 'q') return 0;
	if (c1 == 'n' && c2 == 'e') return 1;
	if (c1 == 'c' && c2 == 's') return 2;
	if (c1 == 'h' && c2 == 's') return 2;
	if (c1 == 'c' && c2 == 'c') return 3;
	if (c1 == 'l' && c2 == 'o') return 3;
	if (c1 == 'm' && c2 == 'i') return 4;
	if (c1 == 'p' && c2 == 'l') return 5;
	if (c1 == 'v' && c2 == 's') return 6;
	if (c1 == 'v' && c2 == 'c') return 7;
	if (c1 == 'h' && c2 == 'i') return 8;
	if (c1 == 'l' && c2 == 's') return 9;
	if (c1 == 'g' && c2 == 'e') return 10;
	if (c1 == 'l' && c2 == 't') return 11;
	if (c1 == 'g' && c2 == 't') return 12;
	if (c1 == 'l' && c2 == 'e') return 13;
	if (c1 == 'a' && c2 == 'l') return 14;

	string -= 2;
	return 14;
}


int ArmGetAddressingMode(char* string)
{
	char c1 = string[0];
	char c2 = string[1];

	if (c1 == 'i' && c2 == 'b') return ARM_AMODE_IB;
	if (c1 == 'i' && c2 == 'a') return ARM_AMODE_IA;
	if (c1 == 'd' && c2 == 'b') return ARM_AMODE_DB;
	if (c1 == 'd' && c2 == 'a') return ARM_AMODE_DA;
	if (c1 == 'e' && c2 == 'd') return ARM_AMODE_ED;
	if (c1 == 'f' && c2 == 'd') return ARM_AMODE_FD;
	if (c1 == 'e' && c2 == 'a') return ARM_AMODE_EA;
	if (c1 == 'f' && c2 == 'a') return ARM_AMODE_FA;
	return -1;
}

int ArmGetShiftMode(char* string)
{
	char c1 = string[0];
	char c2 = string[1];
	char c3 = string[2];

	if (c1 == 'l' && c2 == 's' && c3 == 'l') return 0;	// lsl
	if (c1 == 'l' && c2 == 's' && c3 == 'r') return 1;	// lsr
	if (c1 == 'a' && c2 == 's' && c3 == 'r') return 2;	// asr
	if (c1 == 'r' && c2 == 'o' && c3 == 'r') return 3;	// ror
	if (c1 == 'r' && c2 == 'r' && c3 == 'x') return 4;	// rrx
	return -1;
}


bool CArmInstruction::ParseOpcode(char* Encoding, char* Line)
{
	Vars.Opcode.c = Vars.Opcode.a = 0;
	Vars.Opcode.s = false;

	while (*Encoding != 0)
	{
		switch (*Encoding)
		{
		case 'C':	// condition
			Vars.Opcode.c = ArmGetCondition(Line);
			Encoding++;
			break;
		case 'S':	// set flag
			if (*Line == 's')
			{
				Line++;
				Vars.Opcode.s = true;
			}
			Encoding++;
			break;
		case 'A':	// addressing mode
			if ((Vars.Opcode.a = ArmGetAddressingMode(Line)) == -1) return false;
			Line += 2;
			Encoding++;
			break;
		case 'X':	// x flag
			if (*Line == 't') Vars.Opcode.x = true;
			else if (*Line == 'b') Vars.Opcode.x = false;
			else return false;
			Line++;
			Encoding++;
			break;
		case 'Y':	// y flag
			if (*Line == 't') Vars.Opcode.y = true;
			else if (*Line == 'b') Vars.Opcode.y = false;
			else return false;
			Line++;
			Encoding++;
			break;
		default:
			if (*Encoding++ != *Line++) return false;
			break;
		}
	}

	if (*Line != 0) return false;
	return true;
}

bool CArmInstruction::ParseShift(char*& Line, int mode)
{
	char ImmediateBuffer[512];
	CStringList List;
	int RetLen;

	Vars.Shift.UseShift = false;
	if (*Line != ',') return true;	// a shift has to start with a comma
	while (Line[1] == ' ' || Line[1] == '\t') Line++;
	if ((Vars.Shift.Type = ArmGetShiftMode(&Line[1])) == 0xFF) return false;
	Line += 4;
	while (*Line == ' ' || *Line == '\t') Line++;

	if (ArmGetRegister(Line,RetLen,Vars.Shift.reg) == true)	// shift by register
	{
		if (mode == '1') return false;	// mode 1 can only be an immediate
		Vars.Shift.ShiftByRegister = true;
	} else {	// shift by immediate
		if (*Line == '#') Line++;
		if (ArmCheckImmediate(Line,ImmediateBuffer,RetLen,List) == false) return false;
		Vars.Shift.ShiftByRegister = false;
		if (CheckPostfix(List,true) == false)
		{
			Logger::printError(Logger::Error,L"Invalid shift expression \"%S\"",ImmediateBuffer);
			NoCheckError = true;
			return false;
		}
		Vars.Shift.ShiftExpression.Load(List);
	}
	Line += RetLen;
	Vars.Shift.UseShift = true;
	return true;
}

#define ARMREG_NORMAL	false
#define ARMREG_COP		true

typedef struct {
	char Character;
	bool type;
	std::ptrdiff_t StructOffset;
} tArmRegisterLookup;

const tArmRegisterLookup RegisterLookup[] = {
	{ 'd',	ARMREG_NORMAL,	offsetof(tArmOpcodeVariables,rd) },
	{ 's',	ARMREG_NORMAL,	offsetof(tArmOpcodeVariables,rs) },
	{ 'n',	ARMREG_NORMAL,	offsetof(tArmOpcodeVariables,rn) },
	{ 'm',	ARMREG_NORMAL,	offsetof(tArmOpcodeVariables,rm) },
	{ 'D',	ARMREG_COP,		offsetof(tArmOpcodeVariables,CopData.cd) },
	{ 'N',	ARMREG_COP,		offsetof(tArmOpcodeVariables,CopData.cn) },
	{ 'M',	ARMREG_COP,		offsetof(tArmOpcodeVariables,CopData.cm) },
	{ 0,	0,				-1 }
};
/*
			case 'M':	// cop register m
				if (ArmGetCopRegister(Line,RetLen,Vars.CopData.cm) == false) return false;
				Line += RetLen;
				SourceEncoding++;*/
bool CArmInstruction::LoadEncoding(const tArmOpcode& SourceOpcode, char* Line)
{
	char ImmediateBuffer[512];

	int RetLen;
	CStringList List;
	bool Immediate = false;

	Vars.psr= false;
	Vars.writeback = false;
	Vars.SignPlus = false;
	Vars.Opcode.UseNewEncoding = false;
	Vars.Opcode.UseNewType = false;
		
	char* SourceEncoding = (char*)(SourceOpcode.mask);
	char* OriginalLine = Line;

	while (*Line == ' ' || *Line == '\t') Line++;

	tArmRegisterInfo* Info;
	if (!(*SourceEncoding == 0 && *Line == 0))
	{
		while (*SourceEncoding != 0)
		{
			while (*Line == ' ' || *Line == '\t') Line++;
//			if (*Line == 0) return false;

			switch (*SourceEncoding)
			{
			case 'd': case 's': case 'n': case 'm':
			case 'D': case 'N': case 'M':	// all of them are registers
				for (int i = 0; ; i++)
				{
					if (RegisterLookup[i].Character == *SourceEncoding)
					{
						Info = (tArmRegisterInfo*)((uintptr_t) (&Vars) + RegisterLookup[i].StructOffset);
						if (RegisterLookup[i].type == ARMREG_NORMAL)
						{
							if (ArmGetRegister(Line,RetLen,*Info) == false) return false;
							if (*(SourceEncoding+1) == '1' && Info->Number == 15) return false;
							Line += RetLen;
							SourceEncoding += 2;
						} else {
							if (ArmGetCopRegister(Line,RetLen,*Info) == false) return false;
							Line += RetLen;
							SourceEncoding++;
						}
						break;
					}
				}
				break;
/*			case 'd':	// reg
				if (ArmGetRegister(Line,RetLen,Vars.rd) == false) return false;
				if (*(SourceEncoding+1) == '1' && Vars.rd.Number == 15) return false;
				Line += RetLen;
				SourceEncoding += 2;
				break;
			case 's':	// reg
				if (ArmGetRegister(Line,RetLen,Vars.rs) == false) return false;
				if (*(SourceEncoding+1) == '1' && Vars.rs.Number == 15) return false;
				Line += RetLen;
				SourceEncoding += 2;
				break;
			case 'n':	// reg
				if (ArmGetRegister(Line,RetLen,Vars.rn) == false) return false;
				if (*(SourceEncoding+1) == '1' && Vars.rn.Number == 15) return false;
				Line += RetLen;
				SourceEncoding += 2;
				break;
			case 'm':	// reg
				if (ArmGetRegister(Line,RetLen,Vars.rm) == false) return false;
				if (*(SourceEncoding+1) == '1' && Vars.rm.Number == 15) return false;
				Line += RetLen;
				SourceEncoding += 2;
				break;*/
			case 'W':	// writeback
				if (*Line == '!')
				{
					Vars.writeback = true;
					Line++;
				}
				SourceEncoding++;
				break;
			case 'p':	// psr
				if (*Line == '^')
				{
					Vars.psr = true;
					Line++;
				}
				SourceEncoding++;
				break;
			case 'P':	// msr/mrs psr data
				if (strncmp(Line,"cpsr",4) == 0)	// is cpsr
				{
					Vars.PsrData.spsr = false;
					Line += 4;
				} else if (strncmp(Line,"spsr",4) == 0)	// is spsr
				{
					Vars.PsrData.spsr = true;
					Line += 4;
				} else return false;		// otherwise it's neither

				if (SourceEncoding[1] != '1')
				{
					if (*Line != '_')		// no underscore = short version
					{
						Vars.PsrData.field = 0xF;
					} else {
						Line++;
						if (memcmp(Line,"ctl",3) == 0)
						{
							Vars.PsrData.field = 1;
							Line += 3;
						} else if (memcmp(Line,"flg",3) == 0)
						{
							Vars.PsrData.field = 8;
							Line += 3;
						} else {
							Vars.PsrData.field = 0;
							for (int i = 0; i < 4; i++)
							{
								if (*Line == ',') break;
								if (*Line == 'f')
								{
									if (Vars.PsrData.field & 8) return false;	// can only appear once
									Vars.PsrData.field |= 8;
								} else if (*Line == 's')
								{
									if (Vars.PsrData.field & 4) return false;	// can only appear once
									Vars.PsrData.field |= 4;
								} else if (*Line == 'x')
								{
									if (Vars.PsrData.field & 2) return false;	// can only appear once
									Vars.PsrData.field |= 2;
								} else if (*Line == 'c')
								{
									if (Vars.PsrData.field & 1) return false;	// can only appear once
									Vars.PsrData.field |= 1;
								} else return false;	// has to be one of those
								Line++;
							}
						}
					}
				}
				SourceEncoding+=2;
				break;
			case 'R':	// rlist
				if (ArmGetRlist(Line,RetLen,0xFFFF,Vars.rlist) == false) return false;
				memcpy(Vars.RlistStr,Line,RetLen);
				Vars.RlistStr[RetLen] = 0;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'S':
				if (ParseShift(Line,SourceEncoding[1]) == false) return false;
				SourceEncoding += 2;
				break;
			case 'I':	// immediate
			case 'i':
				if (ArmCheckImmediate(Line,ImmediateBuffer,RetLen,List) == false) return false;
				Line += RetLen;
				Vars.ImmediateBitLen = 32;
				SourceEncoding++;
				break;
			case 'j':	// variable bit immediate
				if (ArmCheckImmediate(Line,ImmediateBuffer,RetLen,List) == false) return false;
				Line += RetLen;
				Vars.ImmediateBitLen = *(SourceEncoding+1);
				SourceEncoding+=2;
				break;
			case '/':	// optional character
				if (*Line == *(SourceEncoding+1)) Line++;
				SourceEncoding += 2;
				break;
/*			case 'D':	// cop register d
				if (ArmGetCopRegister(Line,RetLen,Vars.CopData.cd) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'N':	// cop register n
				if (ArmGetCopRegister(Line,RetLen,Vars.CopData.cn) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'M':	// cop register m
				if (ArmGetCopRegister(Line,RetLen,Vars.CopData.cm) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;*/
			case 'X':	// cop number
				if (ArmGetCopNumber(Line,RetLen,Vars.CopData.pn) == false) return false;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'Y':	// cop opcode number
				if (ArmCheckImmediate(Line,ImmediateBuffer,RetLen,List) == false) return false;
				if (CheckPostfix(List,true) == false)
				{
					Logger::printError(Logger::Error,L"Invalid expression \"%S\"",ImmediateBuffer);
					NoCheckError = true;
					return false;
				}
				Vars.CopData.CpopExpression.Load(List);
				Vars.ImmediateBitLen = 4;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'Z':	// cop info number
				if (ArmCheckImmediate(Line,ImmediateBuffer,RetLen,List) == false) return false;
				if (CheckPostfix(List,true) == false)
				{
					Logger::printError(Logger::Error,L"Invalid expression \"%S\"",ImmediateBuffer);
					NoCheckError = true;
					return false;
				}
				Vars.CopData.CpinfExpression.Load(List);
				Vars.ImmediateBitLen = 3;
				Line += RetLen;
				SourceEncoding++;
				break;
			case 'z':	// shift for pseudo opcodes
				Vars.Shift.Type = SourceEncoding[1];
				if (ArmGetRegister(Line,RetLen,Vars.Shift.reg) == true)	// shift by register
				{
					Vars.Shift.ShiftByRegister = true;
				} else {	// shift by immediate
					if (ArmCheckImmediate(Line,ImmediateBuffer,RetLen,List) == false) return false;
					Vars.Shift.ShiftByRegister = false;
					if (CheckPostfix(List,true) == false)
					{
						Logger::printError(Logger::Error,L"Invalid shift expression \"%S\"",ImmediateBuffer);
						NoCheckError = true;
						return false;
					}
					Vars.Shift.ShiftExpression.Load(List);
				}
				Line += RetLen;
				Vars.Shift.UseShift = true;
				SourceEncoding += 2;
				break;
			case 'v':	// sign for register index parameter
				if (*Line == '-')
				{
					Vars.SignPlus = false;
					Line++;
				} else if (*Line == '+')
				{
					Vars.SignPlus = true;
					Line++;
				} else {
					Vars.SignPlus = true;
				}
				SourceEncoding++;
				break;
			default:	// everything else
				if (*SourceEncoding++ != *Line++) return false;
				break;
			}
		}
	}

	while (*Line == ' ' || *Line == '\t') Line++;
	if (*Line != 0)	return false;	// there's something left, bad

	// opcode is fine - now set all flags
	Opcode = SourceOpcode;

	if (Opcode.flags & ARM_IMMEDIATE)
	{
		if (CheckPostfix(List,true) == false)
		{
			Logger::printError(Logger::Error,L"Invalid expression \"%S\"",ImmediateBuffer);
			NoCheckError = true;
			return false;
		}
		Vars.ImmediateExpression.Load(List);
	}

	return true;
}

bool CArmInstruction::Load(char *Name, char *Params)
{
	bool paramfail = false;
	NoCheckError = false;

	for (int z = 0; ArmOpcodes[z].name != NULL; z++)
	{
		if ((ArmOpcodes[z].flags & ARM_ARM9) && !Arm.isArm9())
			continue;

		if (ParseOpcode((char*)ArmOpcodes[z].name,Name) == true)
		{
			if (LoadEncoding(ArmOpcodes[z],Params) == true)
			{
				Loaded = true;
				return true;
			}
			paramfail = true;
		}
	}

	if (NoCheckError == false)
	{
		if (paramfail == true)
		{
			Logger::printError(Logger::Error,L"ARM parameter failure \"%S\"",Params);
		} else {
			Logger::printError(Logger::Error,L"Invalid ARM opcode \"%S\"",Name);
		}
	}
	return false;
}

bool ParsePostfixExpressionCheck(CExpressionCommandList& Postfix, CStringList* Errors, int& Result)
{
	if (ParsePostfix(Postfix,Errors,Result) == false)
	{
		if (Errors != NULL)	
		{
			if (Errors->GetCount() == 0)
			{
				Logger::queueError(Logger::Error,L"Invalid expression");
			} else {
				for (size_t l = 0; l < Errors->GetCount(); l++)
				{
					Logger::queueError(Logger::Error,convertUtf8ToWString(Errors->GetEntry(l)));
				}
			}
		}
		return false;
	}
	return true;
}

#include <stddef.h>
bool CArmInstruction::Validate()
{
	CStringList List;

	RamPos = g_fileManager->getVirtualAddress();
	g_fileManager->advanceMemory(4);

	Vars.Opcode.UseNewEncoding = false;
	Vars.Opcode.UseNewType = false;


	if (RamPos & 3)
	{
		Logger::queueError(Logger::Warning,L"Opcode not word aligned");
	}

	if (Vars.Shift.UseShift == true && Vars.Shift.ShiftByRegister == false)
	{
		if (ParsePostfixExpressionCheck(Vars.Shift.ShiftExpression,&List,
			Vars.Shift.ShiftAmount) == false) return false;

		int mode = Vars.Shift.Type;
		int num = Vars.Shift.ShiftAmount;

		if (num == 0 && mode == ARM_SHIFT_LSR) mode = ARM_SHIFT_LSL;
		else if (num == 0 && mode == ARM_SHIFT_ASR) mode = ARM_SHIFT_LSL;
		else if (num == 0 && mode == ARM_SHIFT_ROR) mode = ARM_SHIFT_LSL;
		else if (num == 32 && mode == ARM_SHIFT_LSR) num = 0;
		else if (num == 32 && mode == ARM_SHIFT_ASR) num = 0;
		else if (num == 32 && mode == ARM_SHIFT_LSL)
		{
			Logger::queueError(Logger::Error,L"Invalid shift mode");
			return false;
		} else if (mode == ARM_SHIFT_RRX)
		{
			if (num != 1)
			{
				Logger::queueError(Logger::Error,L"Invalid shift mode");
				return false;
			}
			mode = ARM_SHIFT_ROR;
			num = 0;
		}

		if (num > 32 || num < 0)
		{
			Logger::queueError(Logger::Error,L"Shift amount out of range");
			return false;
		}

		Vars.Shift.FinalType = mode;
		Vars.Shift.FinalShiftAmount = num;
		Vars.Shift.UseFinal = true;
	}

	if (Opcode.flags & ARM_COPOP)
	{
		if (ParsePostfixExpressionCheck(Vars.CopData.CpopExpression,&List,Vars.CopData.Cpop) == false)
		{
			return false;
		}
		if (Vars.CopData.Cpop > 15)
		{
			Logger::queueError(Logger::Error,L"CP Opc number %02X too big",Vars.CopData.Cpop);
			return false;
		}
	}

	if (Opcode.flags & ARM_COPINF)
	{
		if (ParsePostfixExpressionCheck(Vars.CopData.CpinfExpression,&List,Vars.CopData.Cpinf) == false)
		{
			return false;
		}
		if (Vars.CopData.Cpinf > 7)
		{
			Logger::queueError(Logger::Error,L"CP Inf number %02X too big",Vars.CopData.Cpinf);
			return false;
		}
	}

	if (Opcode.flags & ARM_DN)
	{
		Vars.rn = Vars.rd;
	}

	if (Opcode.flags & ARM_DM)
	{
		Vars.rm = Vars.rd;
	}

	if (Opcode.flags & ARM_RDEVEN)
	{
		if (Vars.rd.Number & 1)
		{
			Logger::queueError(Logger::Error,L"rd must be even");
			return false;
		}
	}

	if (Opcode.flags & ARM_IMMEDIATE)
	{
		if (ParsePostfixExpressionCheck(Vars.ImmediateExpression,&List,Vars.Immediate) == false)
		{
			return false;
		}
		Vars.OriginalImmediate = Vars.Immediate;
		Vars.negative = false;
			
		if (Opcode.flags & ARM_SHIFT)	// shifted immediate, eg 4000h
		{
			int temp;
			Vars.negative = false;
			if ((Opcode.flags & ARM_ABS) && Vars.Immediate < 0)
			{
				Vars.Immediate = abs(Vars.Immediate);
				Vars.negative = true;
			}

			if (Opcode.flags & ARM_PCR)
			{
				Vars.Immediate = Vars.Immediate - ((RamPos+8) & ~3);
				if (Vars.Immediate < 0)
				{
					Vars.Opcode.NewEncoding = Opcode.encoding ^ 0xC00000;
					Vars.Opcode.UseNewEncoding = true;
					Vars.Immediate = abs(Vars.Immediate);
				}
			}

			if ((temp = ArmGetShiftedImmediate(Vars.Immediate,Vars.Shift.ShiftAmount)) == -1)
			{
				// mov/mvn -> mvn/mov
				if ((Opcode.flags & ARM_OPTIMIZE) && (temp = ArmGetShiftedImmediate(~Vars.Immediate,Vars.Shift.ShiftAmount)) != -1)
				{
					if (Opcode.flags & ARM_OPMOVMVN) Vars.Opcode.NewEncoding = Opcode.encoding ^ 0x0400000;
					if (Opcode.flags & ARM_OPANDBIC) Vars.Opcode.NewEncoding = Opcode.encoding ^ 0x1C00000;
					if (Opcode.flags & ARM_OPCMPCMN) Vars.Opcode.NewEncoding = Opcode.encoding ^ 0x0200000;
					Vars.Opcode.UseNewEncoding = true;
				} else {
					Logger::queueError(Logger::Error,L"Invalid shifted immediate %X",Vars.OriginalImmediate);
					return false;
				}
			}
			Vars.Immediate = temp;
		} else if (Opcode.flags & ARM_POOL)
		{
			int pos, temp;

			if ((temp = ArmGetShiftedImmediate(Vars.Immediate,Vars.Shift.ShiftAmount)) != -1)
			{
				// interpete ldr= as mov
				Vars.Opcode.NewEncoding = 0x03A00000;
				Vars.Opcode.UseNewEncoding = true;
				Vars.Opcode.NewType = ARM_TYPE5;
				Vars.Opcode.UseNewType = true;
				Vars.Immediate = temp;
			} else if ((temp = ArmGetShiftedImmediate(~Vars.Immediate,Vars.Shift.ShiftAmount)) != -1) 
			{
				// interprete ldr= as mvn
				Vars.Opcode.NewEncoding = 0x03E00000;
				Vars.Opcode.UseNewEncoding = true;
				Vars.Opcode.NewType = ARM_TYPE5;
				Vars.Opcode.UseNewType = true;
				Vars.Immediate = temp;
			} else if ((pos = Arm.AddToCurrentPool(Vars.Immediate)) == -1)
			{
				Logger::queueError(Logger::Error,L"Unable to find literal pool");
				return false;
			} else {
				pos = pos-((RamPos+8) & 0xFFFFFFFD);
				if (abs(pos) > 4095)
				{
					Logger::queueError(Logger::Error,L"Literal pool out of range");
					return false;
				}
				Vars.Immediate = pos;
			}
		} else if (Opcode.flags & ARM_BRANCH)
		{
			if (Opcode.flags & ARM_HALFWORD)
			{
				if (Vars.Immediate & 1)
				{
					Logger::queueError(Logger::Error,L"Branch target must be halfword aligned");
					return false;
				}
			} else {
				if (Vars.Immediate & 3)
				{
					Logger::queueError(Logger::Error,L"Branch target must be word aligned");
					return false;
				}
			}

			Vars.Immediate = (int) (Vars.Immediate-RamPos-8);
			if (abs(Vars.Immediate) >= 0x2000000)
			{
				Logger::queueError(Logger::Error,L"Branch target %08X out of range",Vars.OriginalImmediate);
				return false;
			}
		} else if (Opcode.flags & ARM_ABSIMM)	// ldr r0,[I]
		{
			Vars.Immediate = (int) (Vars.Immediate-RamPos-8);
			if (abs(Vars.Immediate) >= (1 << Vars.ImmediateBitLen))
			{
				Logger::queueError(Logger::Error,L"Load target %08X out of range",Vars.OriginalImmediate);
				return false;
			}
		} else if (Opcode.flags & ARM_SWI)	// it's an interrupt, may need to shift it
		{
			if (Vars.Immediate < 0xFF)
			{
				Vars.Immediate <<= 16;
				Vars.OriginalImmediate = Vars.Immediate;
			}
		} else if ((Opcode.flags & ARM_ABS) && Vars.Immediate < 0)
		{
			Vars.Immediate = abs(Vars.Immediate);
			Vars.negative = true;
		}

		if (Vars.ImmediateBitLen != 32 && !(Opcode.flags & ARM_ABSIMM))
		{
			unsigned int check = Opcode.flags & ARM_ABS ? abs(Vars.Immediate) : Vars.Immediate;
			if (check >= (unsigned int)(1 << Vars.ImmediateBitLen))
			{
				Logger::queueError(Logger::Error,L"Immediate value %X out of range",Vars.Immediate);
				return false;
			}
		}
	}

	return false;
}

void CArmInstruction::FormatOpcode(char* Dest, const char* Source)
{
	while (*Source != 0)
	{
		switch (*Source)
		{
		case 'C':	// condition
			Dest += sprintf(Dest,"%s",ArmConditions[Vars.Opcode.c]);
			Source++;
			break;
		case 'S':	// set flag
			if (Vars.Opcode.s == true) *Dest++ = 's';
			Source++;
			break;
		case 'A':	// addressing mode
			if (Opcode.flags & ARM_LOAD)
			{
				Dest += sprintf(Dest,"%s",ArmAddressingModes[LdmModes[Vars.Opcode.a]]);
			} else {
				Dest += sprintf(Dest,"%s",ArmAddressingModes[StmModes[Vars.Opcode.a]]);
			}
			Source++;
			break;
		case 'X':	// x flag
			if (Vars.Opcode.x == false) *Dest++ = 'b';
			else *Dest++ = 't';
			Source++;
			break;
		case 'Y':	// y flag
			if (Vars.Opcode.y == false) *Dest++ = 'b';
			else *Dest++ = 't';
			Source++;
			break;
		default:
			*Dest++ = *Source++;
			break;
		}
	}
	*Dest = 0;
}

void CArmInstruction::FormatInstruction(const char* encoding, char* dest)
{
	while (*encoding != 0)
	{
		switch (*encoding)
		{
		case 's':	//  reg
			dest += sprintf(dest,"%s",Vars.rs.Name);
			encoding += 2;
			break;
		case 'd':	//  reg
			dest += sprintf(dest,"%s",Vars.rd.Name);
			encoding += 2;
			break;
		case 'n':	//  reg
			dest += sprintf(dest,"%s",Vars.rn.Name);
			encoding += 2;
			break;
		case 'm':	//  reg
			dest += sprintf(dest,"%s",Vars.rm.Name);
			encoding += 2;
			break;
		case 'W':	// writeback
			if (Vars.writeback == true) *dest++ = '!';
			encoding++;
			break;
		case 'p':	// psr
			if (Vars.psr == true) *dest++ = '^';
			encoding++;
			break;
		case 'P':	// mrs/msr psr
			dest += sprintf(dest,"%s",Vars.PsrData.spsr == true ? "spsr" : "cpsr");
			if (encoding[1] == '0') dest += sprintf(dest,"%s",ArmPsrModes[Vars.PsrData.field]);
			encoding += 2;
			break;
		case 'S':	// shifts
			if (Vars.Shift.UseShift == true)
			{
				*dest++ = ',';
				dest += sprintf(dest,"%s ",ArmShiftModes[Vars.Shift.Type]);
				if (Vars.Shift.ShiftByRegister == true)
				{
					dest += sprintf(dest,"%s",Vars.Shift.reg.Name);
				} else {
					dest += sprintf(dest,"0x%X",Vars.Shift.ShiftAmount);
				}
			}
			encoding += 2;
			break;
		case 'R':	// rlist
			dest += sprintf(dest,"%s",Vars.RlistStr);
			encoding++;
			break;
		case 'I':
		case 'i':
			dest += sprintf(dest,"0x%08X",Vars.OriginalImmediate);
			encoding++;
			break;
		case 'j':
			dest += sprintf(dest,"0x%0*X",(Vars.ImmediateBitLen+3)>>2,Vars.OriginalImmediate & ((1 << Vars.ImmediateBitLen)-1));
			encoding+=2;
			break;
		case 'D':	// cop reg
			dest += sprintf(dest,"%s",Vars.CopData.cd.Name);
			encoding++;
			break;
		case 'N':	// cop reg
			dest += sprintf(dest,"%s",Vars.CopData.cn.Name);
			encoding++;
			break;
		case 'M':	// cop reg
			dest += sprintf(dest,"%s",Vars.CopData.cm.Name);
			encoding++;
			break;
		case 'X':	// cop number
			dest += sprintf(dest,"%s",Vars.CopData.pn.Name);
			encoding++;
			break;
		case 'Y':	// cop opcode
			dest += sprintf(dest,"0x%02X",Vars.CopData.Cpop);
			encoding++;
			break;
		case 'Z':	// cop inf
			dest += sprintf(dest,"0x%02X",Vars.CopData.Cpinf);
			encoding++;
			break;
		case 'v':	// sign
			if (Vars.SignPlus == false) dest += sprintf(dest,"-");
			encoding++;
			break;
		case '/':
			encoding += 2;
			break;
		default:
			*dest++ = *encoding++;
			break;
		}
	}
	*dest = 0;
}

void CArmInstruction::writeTempData(TempData& tempData)
{
	char OpcodeName[32];
	char str[256];

	FormatOpcode(OpcodeName,Opcode.name);
	int pos = sprintf(str,"   %s",OpcodeName);
	while (pos < 11) str[pos++] = ' ';
	str[pos] = 0;
	FormatInstruction(Opcode.mask,&str[pos]);

	tempData.writeLine(RamPos,convertUtf8ToWString(str));
}

void CArmInstruction::WriteInstruction(unsigned int encoding)
{
	g_fileManager->write(&encoding,4);
}

void CArmInstruction::Encode()
{
	unsigned int encoding = Vars.Opcode.UseNewEncoding == true ? Vars.Opcode.NewEncoding : Opcode.encoding;

	if ((Opcode.flags & ARM_UNCOND) == 0) encoding |= Vars.Opcode.c << 28;
	if (Vars.Opcode.s == true) encoding |= (1 << 20);

	if (Vars.Shift.UseFinal == true)
	{
		Vars.Shift.Type = Vars.Shift.FinalType;
		Vars.Shift.ShiftAmount = Vars.Shift.FinalShiftAmount;
	}


	switch (Vars.Opcode.UseNewType == true ? Vars.Opcode.NewType : Opcode.type)
	{
	case ARM_TYPE3:		// ARM.3: Branch and Exchange (BX, BLX)
		encoding |= (Vars.rn.Number << 0);
		break;
	case ARM_TYPE4:		// ARM.4: Branch and Branch with Link (B, BL, BLX)
		if ((Opcode.flags & ARM_HALFWORD) && (Vars.Immediate & 2)) encoding |= 1 << 24;
		encoding |= (Vars.Immediate >> 2) & 0xFFFFFF;
		break;
	case ARM_TYPE5:		// ARM.5: Data Processing
		if (Opcode.flags & ARM_N) encoding |= (Vars.rn.Number << 16);
		if (Opcode.flags & ARM_D) encoding |= (Vars.rd.Number << 12);

		if (Opcode.flags & ARM_IMMEDIATE)	// immediate als op2
		{
			encoding |= (Vars.Shift.ShiftAmount << 7);
			encoding |= Vars.Immediate;
		} else if (Opcode.flags & ARM_REGISTER) {	// shifted register als op2
			if (Vars.Shift.UseShift == true)
			{
				if (Vars.Shift.ShiftByRegister == true)
				{
					encoding |= (Vars.Shift.reg.Number << 8);
					encoding |= (1 << 4);
				} else {	// shiftbyimmediate
					encoding |= (Vars.Shift.ShiftAmount << 7);
				}
				encoding |= (Vars.Shift.Type << 5);
			}
			encoding |= (Vars.rm.Number << 0);
		}
		break;
	case ARM_TYPE6:		// ARM.6: PSR Transfer (MRS, MSR)
		if (Opcode.flags & ARM_MRS) //  MRS{cond} Rd,Psr          ;Rd = Psr
		{
			if (Vars.PsrData.spsr == true) encoding |= (1 << 22);
			encoding |= (Vars.rd.Number << 12);
		} else {					//  MSR{cond} Psr{_field},Op  ;Psr[field] = Op
			if (Vars.PsrData.spsr == true) encoding |= (1 << 22);
			encoding |= (Vars.PsrData.field << 16);

			if (Opcode.flags & ARM_REGISTER)
			{
				encoding |= (Vars.rm.Number << 0);
			} else if (Opcode.flags & ARM_IMMEDIATE)
			{
				encoding |= (Vars.Shift.ShiftAmount << 7);
				encoding |= Vars.Immediate;
			}
		}
		break;
	case ARM_TYPE7:		// ARM.7: Multiply and Multiply-Accumulate (MUL,MLA)
		encoding |= (Vars.rd.Number << 16);
		if (Opcode.flags & ARM_N) encoding |= (Vars.rn.Number << 12);
		encoding |= (Vars.rs.Number << 8);
		if ((Opcode.flags & ARM_Y) && Vars.Opcode.y == true) encoding |= (1 << 6);
		if ((Opcode.flags & ARM_X) && Vars.Opcode.x == true) encoding |= (1 << 5);
		encoding |= (Vars.rm.Number << 0);
		break;
	case ARM_TYPE9:		// ARM.9: Single Data Transfer (LDR, STR, PLD)
		if (Vars.writeback == true) encoding |= (1 << 21);
		if (Opcode.flags & ARM_N) encoding |= (Vars.rn.Number << 16);
		if (Opcode.flags & ARM_D) encoding |= (Vars.rd.Number << 12);
		if ((Opcode.flags & ARM_SIGN) && Vars.SignPlus == false) encoding &= ~(1 << 23);
		if ((Opcode.flags & ARM_ABS) && Vars.negative == true) encoding &= ~(1 << 23);
		if (Opcode.flags & ARM_IMMEDIATE)
		{
			if (Vars.Immediate < 0)
			{
				encoding &= ~(1 << 23);
				Vars.Immediate = abs(Vars.Immediate);
			}
			encoding |= (Vars.Immediate << 0);
		} else if (Opcode.flags & ARM_REGISTER)	// ... heißt der opcode nutzt shifts, mit immediates
		{
			if (Vars.Shift.UseShift == true)
			{
				encoding |= (Vars.Shift.ShiftAmount << 7);
				encoding |= (Vars.Shift.Type << 5);
			}
			encoding |= (Vars.rm.Number << 0);
		}
		break;
	case ARM_TYPE10:	// ARM.10: Halfword, Doubleword, and Signed Data Transfer
		if (Vars.writeback == true) encoding |= (1 << 21);
		encoding |= (Vars.rn.Number << 16);
		encoding |= (Vars.rd.Number << 12);
		if ((Opcode.flags & ARM_SIGN) && Vars.SignPlus == false) encoding &= ~(1 << 23);
		if ((Opcode.flags & ARM_ABS) && Vars.negative == true) encoding &= ~(1 << 23);
		if (Opcode.flags & ARM_IMMEDIATE)
		{
			if (Vars.Immediate < 0)
			{
				encoding &= ~(1 << 23);
				Vars.Immediate = abs(Vars.Immediate);
			}
			encoding |= ((Vars.Immediate & 0xF0) << 4);
			encoding |= (Vars.Immediate & 0xF);
		} else if (Opcode.flags & ARM_REGISTER)
		{
			encoding |= (Vars.rm.Number << 0);
		}
		break;
	case ARM_TYPE11:	// ARM.11: Block Data Transfer (LDM,STM)
		if (Opcode.flags & ARM_LOAD) encoding |= (LdmModes[Vars.Opcode.a] << 23);
		else if (Opcode.flags & ARM_STORE) encoding |= (StmModes[Vars.Opcode.a] << 23);
		if (Vars.psr == true) encoding |= (1 << 22);
		if (Vars.writeback == true) encoding |= (1 << 21);
		if (Opcode.flags & ARM_N) encoding |= (Vars.rn.Number << 16);
		encoding |= (Vars.rlist);
		break;
	case ARM_TYPE12:	// ARM.12: Single Data Swap (SWP)
	case ARM_MISC:		// ARM.X: Count Leading Zeros
		encoding |= (Vars.rm.Number << 0);
		encoding |= (Vars.rd.Number << 12);
		if (Opcode.flags & ARM_N) encoding |= (Vars.rn.Number << 16);
		break;
	case ARM_TYPE13:	// ARM.13: Software Interrupt (SWI,BKPT)
		if (Opcode.flags & ARM_SWI)
		{
			encoding |= Vars.Immediate;
		} else {
			encoding |= (Vars.Immediate & 0xF);
			encoding |= (Vars.Immediate >> 4) << 8;
		}
		break;
	case ARM_TYPE14:	// ARM.14: Coprocessor Data Operations (CDP)
		if (Opcode.flags & ARM_COPOP) encoding |= (Vars.CopData.Cpop << 20);
		encoding |= (Vars.CopData.cn.Number << 16);
		encoding |= (Vars.CopData.cd.Number << 12);
		encoding |= (Vars.CopData.pn.Number << 8);
		if (Opcode.flags & ARM_COPINF) encoding |= (Vars.CopData.Cpinf << 5);
		encoding |= (Vars.CopData.cm.Number << 0);
		break;
	case ARM_TYPE16:	// ARM.16: Coprocessor Register Transfers (MRC, MCR)
		if (Opcode.flags & ARM_COPOP) encoding |= (Vars.CopData.Cpop << 21);
		encoding |= (Vars.CopData.cn.Number << 16);
		encoding |= (Vars.rd.Number << 12);
		encoding |= (Vars.CopData.pn.Number << 8);
		if (Opcode.flags & ARM_COPINF) encoding |= (Vars.CopData.Cpinf << 5);
		encoding |= (Vars.CopData.cm.Number << 0);
		break;
	case ARM_TYPE17:	// ARM.X: Coprocessor Double-Register Transfer (MCRR,MRRC)
		encoding |= (Vars.rn.Number << 16);
		encoding |= (Vars.rd.Number << 12);
		encoding |= (Vars.CopData.pn.Number << 8);
		encoding |= (Vars.CopData.Cpop << 4);
		encoding |= (Vars.CopData.cm.Number << 0);
		break;
	default:
		printf("doh");
	}

	WriteInstruction(encoding);
}
