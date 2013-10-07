#include "stdafx.h"
#include "Core/Directives.h"
#include "Core/Common.h"
#include "Commands/CDirectiveFile.h"
#include "Commands/CDirectiveData.h"
#include "Commands/CDirectiveConditional.h"
#include "Commands/CDirectiveMessage.h"
#include "Commands/CDirectiveFill.h"
#include "Commands/CDirectiveArea.h"
#include "Commands/CAssemblerLabel.h"
#include "Assembler.h"
#include "Archs/MIPS/Mips.h"
#include "Archs/ARM/Arm.h"
#include "Archs/Z80/Z80.h"
#include "Core/MathParser.h"
#include "Util/Util.h"

bool DirectiveFile(CArgumentList& List, int flags)
{
	eDirectiveFileMode value;

	switch (flags & DIRECTIVE_PARAMFIELD)
	{
	case DIRECTIVE_OPEN:
		if (List.GetCount() == 3) value = DIRECTIVEFILE_COPY; else value = DIRECTIVEFILE_OPEN;
		break;
	case DIRECTIVE_CREATE:		value = DIRECTIVEFILE_CREATE; break;
	case DIRECTIVE_CLOSE:		value = DIRECTIVEFILE_CLOSE; break;
	case DIRECTIVE_ORG:			value = DIRECTIVEFILE_ORG; break;
	case DIRECTIVE_ORGA:		value = DIRECTIVEFILE_ORGA; break;
	case DIRECTIVE_INCBIN:		value = DIRECTIVEFILE_INCBIN; break;
	case DIRECTIVE_ALIGN:		value = DIRECTIVEFILE_ALIGN; break;
	case DIRECTIVE_HEADERSIZE:	value = DIRECTIVEFILE_HEADERSIZE; break;
	default:					return false;
	}

	CDirectiveFile* File = new CDirectiveFile(value,List);
	AddAssemblerCommand(File);
	return true;
}

bool DirectiveInclude(CArgumentList& List, int flags)
{
	char FileName[255];
	getFullPathName(FileName,List.GetEntry(0));

	int FileNum = Global.FileInfo.FileNum;
	int LineNum = Global.FileInfo.LineNumber;
	if (fileExists(FileName) == false)
	{
		PrintError(ERROR_ERROR,"Included file \"%s\" does not exist",FileName);
		return false;
	}
	LoadAssemblyFile(FileName);
	Global.FileInfo.FileNum = FileNum;
	Global.FileInfo.LineNumber = LineNum;
	return true;
}

bool DirectiveData(CArgumentList& List, int flags)
{
	int DataSize;
	bool ascii = false;

	switch (flags & (DIRECTIVE_DATA8|DIRECTIVE_DATA16|DIRECTIVE_DATA32))
	{
	case DIRECTIVE_DATA8:	DataSize = 1; break;
	case DIRECTIVE_DATA16:	DataSize = 2; break;
	case DIRECTIVE_DATA32:	DataSize = 4; break;
	default: PrintError(ERROR_ERROR,"Invalid data directive"); return false;;
	}

	if (flags & DIRECTIVE_ASCII) ascii = true;

	CDirectiveData* Data = new CDirectiveData(List,DataSize,ascii);
	AddAssemblerCommand(Data);
	return true;
}

bool DirectiveRadix(CArgumentList& List, int flags)
{
	int rad;
	if (ConvertExpression(List.GetEntry(0),rad) == false)
	{
		PrintError(ERROR_ERROR,"Invalid expression %s",List.GetEntry(0));
		return false;
	}

	switch (rad)
	{
	case 2: case 8: case 10: case 16:
		Global.Radix = rad;
		break;
	default:
		PrintError(ERROR_ERROR,"Invalid radix %d",rad);
		return false;;
	}
	return true;
}

bool DirectiveLoadTable(CArgumentList& List, int flags)
{
	char FileName[255];
	getFullPathName(FileName,List.GetEntry(0));

	if (fileExists(FileName) == false)
	{
		PrintError(ERROR_ERROR,"Table file \"%s\" does not exist",FileName);
		return false;
	}
	if (Global.Table.load(FileName) == false)
	{
		PrintError(ERROR_ERROR,"Invalid table file \"%s\"",FileName);
		return false;
	}
	return true;
}

bool DirectiveString(CArgumentList& List, int flags)
{
	char str[32];
	CArgumentList NewList;

	if (Global.Table.isLoaded() == false)
	{
		PrintError(ERROR_ERROR,"No table opened");
		return false;
	}

	for (int i = 0; i < List.GetCount(); i++)
	{
		char* Input = List.GetEntry(i);

		if (List.IsString(i) == true)
		{
			ByteArray data = Global.Table.encodeString(convertUtf8ToWString(Input),false);
			
			for (int i = 0; i < data.size(); i++)
			{
				sprintf(str,"0x%02X",data[i]);
				NewList.AddEntry(str,false);
			}
		} else {
			NewList.AddEntry(List.GetEntry(i),false);
		}
	}

	ByteArray data = Global.Table.encodeTermination();
	for (int i = 0; i < data.size(); i++)
	{
		sprintf(str,"0x%02X",data[i]);
		NewList.AddEntry(str,false);
	}

	CDirectiveData* Data = new CDirectiveData(NewList,1,false);
	AddAssemblerCommand(Data);
	return true;
}

bool DirectivePsx(CArgumentList& List, int flags)
{
	Arch = &Mips;
	Mips.SetLoadDelay(false,0);
	Mips.SetVersion(MARCH_PSX);
	return true;
}

bool DirectivePs2(CArgumentList& List, int flags)
{
	Arch = &Mips;
	Mips.SetLoadDelay(false,0);
	Mips.SetVersion(MARCH_PS2);
	return true;
}

bool DirectivePsp(CArgumentList& List, int flags)
{
	Arch = &Mips;
	Mips.SetLoadDelay(false,0);
	Mips.SetVersion(MARCH_PSP);
	return true;
}

bool DirectiveGba(CArgumentList& List, int flags)
{
	Arch = &Arm;
	Arm.SetThumbMode(true);
	Arm.SetArm9(false);
	
	ArmStateCommand* cmd = new ArmStateCommand(false);
	AddAssemblerCommand(cmd);
	return true;
}

bool DirectiveNds(CArgumentList& List, int flags)
{
	Arch = &Arm;
	Arm.SetThumbMode(false);
	Arm.SetArm9(true);

	ArmStateCommand* cmd = new ArmStateCommand(true);
	AddAssemblerCommand(cmd);
	return true;
}

bool DirectiveGb(CArgumentList& List, int flags)
{
	Arch = &z80;
	return true;
}

bool DirectiveNocash(CArgumentList& List, int flags)
{
	if (List.GetCount() == 1)
	{
		if (strcmp(List.GetEntry(0),"on") == 0)
		{
			Global.nocash = true;
		} else if (strcmp(List.GetEntry(0),"off") == 0)
		{
			Global.nocash = false;
		} else {
			PrintError(ERROR_ERROR,"Invalid arguments");
			return false;
		}
	} else {
		Global.nocash = true;
	}
	return true;
}

bool DirectiveFill(CArgumentList& List, int flags)
{
	CDirectiveFill* Command = new CDirectiveFill();
	if (Command->Load(List) == false) return false;
	AddAssemblerCommand(Command);
	return true;
}

bool DirectiveDefineLabel(CArgumentList& List, int flags)
{
	int value;
	CAssemblerLabel* labelCommand;

	if (ConvertExpression(List.GetEntry(1),value) == false)
	{
		PrintError(ERROR_ERROR,"Invalid expression \"%s\"",List.GetEntry(1));
		return false;
	}
	
	Label* label = Global.symbolTable.getLabel(convertUtf8ToWString(List.GetEntry(0)),Global.FileInfo.FileNum,Global.Section);
	if (label == NULL)
	{
		PrintError(ERROR_ERROR,"Invalid label name \"%s\"",List.GetEntry(0));
		return false;
	}

	if (label->isDefined())
	{
		PrintError(ERROR_ERROR,"Label \"%s\" already defined",List.GetEntry(0));
		return false;
	}
	
	labelCommand = new CAssemblerLabel(label->getName(),value,Global.Section,true);
	AddAssemblerCommand(labelCommand);
	return true;
}

bool DirectiveConditional(CArgumentList& List, int flags)
{
	int value;

	switch (flags & DIRECTIVE_PARAMFIELD)
	{
	case DIRECTIVE_IF:			value = CONDITIONAL_IF; break;
	case DIRECTIVE_ELSE:		value = CONDITIONAL_ELSE; break;
	case DIRECTIVE_ELSEIF:		value = CONDITIONAL_ELSEIF; break;
	case DIRECTIVE_ENDIF:		value = CONDITIONAL_ENDIF; break;
	case DIRECTIVE_IFDEF:		value = CONDITIONAL_IFDEF; break;
	case DIRECTIVE_IFNDEF:		value = CONDITIONAL_IFNDEF; break;
	case DIRECTIVE_ELSEIFDEF:	value = CONDITIONAL_ELSEIFDEF; break;
	case DIRECTIVE_ELSEIFNDEF:	value = CONDITIONAL_ELSEIFNDEF; break;
	case DIRECTIVE_IFARM:		value = CONDITIONAL_IFARM; break;
	case DIRECTIVE_IFTHUMB:		value = CONDITIONAL_IFTHUMB; break;
	default: return false;
	}

	CDirectiveConditional* Command = new CDirectiveConditional();
	if (Command->Load(List,value) == false)
	{
		delete Command;
		return false;
	}

	AddAssemblerCommand(Command);
	return true;
}

bool DirectiveArea(CArgumentList& List, int flags)
{
	CDirectiveArea* Command;

	switch (flags & DIRECTIVE_PARAMFIELD)
	{
	case DIRECTIVE_AREASTART:
		Command = new CDirectiveArea();
		if (Command->LoadStart(List) == false)
		{
			delete Command;
			return false;
		}
		break;
	case DIRECTIVE_AREAEND:
		Command = new CDirectiveArea();
		if (Command->LoadEnd() == false)
		{
			delete Command;
			return false;
		}
		break;
	default: return false;
	}

	AddAssemblerCommand(Command);
	return true;
}


bool DirectiveMessage(CArgumentList& List, int flags)
{
	int value;

	switch (flags & DIRECTIVE_PARAMFIELD)
	{
	case DIRECTIVE_WARNING:		value = USERMESSAGE_WARNING; break;
	case DIRECTIVE_ERROR:		value = USERMESSAGE_ERROR; break;
	case DIRECTIVE_NOTICE:		value = USERMESSAGE_NOTICE; break;
	default: return false;
	}

	CDirectiveMessage* Command = new CDirectiveMessage();
	if (Command->Load(List,value) == false)
	{
		delete Command;
		return false;
	}

	AddAssemblerCommand(Command);
	return true;
}


bool DirectiveWarningAsError(CArgumentList& List, int flags)
{
	if (List.GetCount() == 1)
	{
		if (strcmp(List.GetEntry(0),"on") == 0)
		{
			Global.warningAsError = true;
		} else if (strcmp(List.GetEntry(0),"off") == 0)
		{
			Global.warningAsError = false;
		} else {
			PrintError(ERROR_ERROR,"Invalid arguments");
			return false;
		}
	} else {
		Global.warningAsError = true;
	}
	return true;
}

bool DirectiveRelativeInclude(CArgumentList& List, int flags)
{
	if (List.GetCount() == 1)
	{
		if (strcmp(List.GetEntry(0),"on") == 0)
		{
			Global.relativeInclude = true;
		} else if (strcmp(List.GetEntry(0),"off") == 0)
		{
			Global.relativeInclude = false;
		} else {
			PrintError(ERROR_ERROR,"Invalid arguments");
			return false;
		}
	} else {
		Global.relativeInclude = true;
	}
	return true;
}

bool DirectiveSym(CArgumentList& List, int flags)
{
	if (strcmp(List.GetEntry(0),"on") == 0)
	{
		CDirectiveSym* sym = new CDirectiveSym(true);
		AddAssemblerCommand(sym);
	} else if (strcmp(List.GetEntry(0),"off") == 0)
	{
		CDirectiveSym* sym = new CDirectiveSym(false);
		AddAssemblerCommand(sym);
	} else {
		PrintError(ERROR_ERROR,"Invalid arguments");
		return false;
	}
	return true;
}

bool SplitArguments(CArgumentList& List, char* args)
{
	char Buffer[2048];
	int BufferPos = 0;
	bool String = false;

	List.Clear();

	while (*args != 0)
	{
		while (*args == ' ' || *args == '\t') args++;
		if (*args == ',')
		{
			if (BufferPos == 0)
			{
				PrintError(ERROR_ERROR,"Parameter failure (empty argument)");
				return false;
			}
			Buffer[BufferPos] = 0;
			List.AddEntry(Buffer,String);
			BufferPos = 0;
			args++;
			String = false;
			continue;
		}

		if (*args == '\\' && *(args+1) == '"')
		{
			Buffer[BufferPos++] = '"';
			args += 2;
			continue;
		}

		if (*args == '"')
		{
			args++;
			while (*args != '"')
			{
				if (*args == '\\' && *(args+1) == '\\')
				{
					Buffer[BufferPos++] = '\\';
					args += 2;
					continue;
				}

				if (*args == '\\' && *(args+1) == '"')
				{
					Buffer[BufferPos++] = '"';
					args += 2;
					continue;
				}

				if (*args == 0)
				{
					PrintError(ERROR_ERROR,"Unexpected end of line in string");
					return false;
				}
				Buffer[BufferPos++] = *args++;
			}
			String = true;
			args++;
			continue;
		}
		Buffer[BufferPos++] = *args++;
		if (BufferPos == 2048)
		{
			PrintError(ERROR_ERROR,"equ replacement length overflow");
			return false;
		}
	}

	if (BufferPos != 0 || String == true)
	{
		Buffer[BufferPos] = 0;
		List.AddEntry(Buffer,String);
	}
	return true;
}

bool ExecuteDirective(const tDirective& Directive, char* args)
{
	CArgumentList Arguments;
	if (SplitArguments(Arguments,args) == false) return false;

	if (Directive.minparam > Arguments.GetCount())
	{
		PrintError(ERROR_ERROR,"Not enough parameters (min %d)",
			Directive.minparam);
		return false;
	}

	if (Directive.maxparam < Arguments.GetCount())
	{
		PrintError(ERROR_ERROR,"Too many parameters (max %d)",
			Directive.maxparam);
		return false;
	}

	if (Directive.Flags & DIRECTIVE_MIPSRESETDELAY)
	{
		Arch->NextSection();
	}

	return Directive.Function(Arguments,Directive.Flags);
}

int GetDirective(const tDirective* DirectiveSet, char* name)
{
	int num = -1;
	for (int z = 0; DirectiveSet[z].name != NULL; z++)
	{
		if (DirectiveSet[z].Flags & DIRECTIVE_DISABLED) continue;
		if (DirectiveSet[z].Flags & DIRECTIVE_NOCASHOFF && Global.nocash == true) continue;
		if (DirectiveSet[z].Flags & DIRECTIVE_NOCASHON && Global.nocash == false) continue;
		if (strcmp(name,DirectiveSet[z].name) == 0)
		{
			num = z;
			break;
		}
	}
	return num;
}

bool DirectiveAssemble(const tDirective* DirectiveSet, char* Name, char* Arguments)
{
	int num = GetDirective(DirectiveSet,Name);
	if (num == -1) return false;
	ExecuteDirective(DirectiveSet[num],Arguments);
	return true;
}


const tDirective Directives[] = {
	{ ".open",		2,	3,	&DirectiveFile,		DIRECTIVE_OPEN },
	{ ".openfile",	2,	3,	&DirectiveFile,		DIRECTIVE_OPEN },
	{ ".create",	2,	2,	&DirectiveFile,		DIRECTIVE_CREATE },
	{ ".createfile",2,	2,	&DirectiveFile,		DIRECTIVE_CREATE },
	{ ".close",		0,	0,	&DirectiveFile,		DIRECTIVE_CLOSE },
	{ ".closefile",	0,	0,	&DirectiveFile,		DIRECTIVE_CLOSE },
	{ ".incbin",	1,	1,	&DirectiveFile,		DIRECTIVE_INCBIN },
	{ ".import",	1,	1,	&DirectiveFile,		DIRECTIVE_INCBIN },
	{ ".org",		1,	1,	&DirectiveFile,		DIRECTIVE_ORG },
	{ "org",		1,	1,	&DirectiveFile,		DIRECTIVE_ORG },
	{ ".orga",		1,	1,	&DirectiveFile,		DIRECTIVE_ORGA },
	{ "orga",		1,	1,	&DirectiveFile,		DIRECTIVE_ORGA },
	{ ".align",		0,	1,	&DirectiveFile,		DIRECTIVE_ALIGN },
	{ ".headersize",1,	1,	&DirectiveFile,		DIRECTIVE_HEADERSIZE },

	{ ".fill",		1,	2,	&DirectiveFill,		0 },
	{ "defs",		1,	2,	&DirectiveFill,		0 },

	{ ".byte",		1,	256,&DirectiveData,		DIRECTIVE_DATA8 },
	{ ".halfword",	1,	256,&DirectiveData,		DIRECTIVE_DATA16 },
	{ ".word",		1,	256,&DirectiveData,		DIRECTIVE_DATA32 },
	{ ".db",		1,	256,&DirectiveData,		DIRECTIVE_DATA8 },
	{ ".dh",		1,	256,&DirectiveData,		DIRECTIVE_DATA16|DIRECTIVE_NOCASHOFF },
	{ ".dw",		1,	256,&DirectiveData,		DIRECTIVE_DATA32|DIRECTIVE_NOCASHOFF },
	{ ".dw",		1,	256,&DirectiveData,		DIRECTIVE_DATA16|DIRECTIVE_NOCASHON },
	{ ".dd",		1,	256,&DirectiveData,		DIRECTIVE_DATA32|DIRECTIVE_NOCASHON },
	{ ".dcb",		1,	256,&DirectiveData,		DIRECTIVE_DATA8 },
	{ ".dcw",		1,	256,&DirectiveData,		DIRECTIVE_DATA16 },
	{ ".dcd",		1,	256,&DirectiveData,		DIRECTIVE_DATA32 },
	{ "db",			1,	256,&DirectiveData,		DIRECTIVE_DATA8 },
	{ "dh",			1,	256,&DirectiveData,		DIRECTIVE_DATA16|DIRECTIVE_NOCASHOFF },
	{ "dw",			1,	256,&DirectiveData,		DIRECTIVE_DATA32|DIRECTIVE_NOCASHOFF },
	{ "dw",			1,	256,&DirectiveData,		DIRECTIVE_DATA16|DIRECTIVE_NOCASHON },
	{ "dd",			1,	256,&DirectiveData,		DIRECTIVE_DATA32|DIRECTIVE_NOCASHON },
	{ "dcb",		1,	256,&DirectiveData,		DIRECTIVE_DATA8 },
	{ "dcw",		1,	256,&DirectiveData,		DIRECTIVE_DATA16 },
	{ "dcd",		1,	256,&DirectiveData,		DIRECTIVE_DATA32 },

	{ ".ascii",		1,	256,&DirectiveData,		DIRECTIVE_DATA8|DIRECTIVE_ASCII },

	{ ".if",		1,	1,	&DirectiveConditional,	DIRECTIVE_IF },
	{ ".else",		0,	0,	&DirectiveConditional,	DIRECTIVE_ELSE },
	{ ".elseif",	1,	1,	&DirectiveConditional,	DIRECTIVE_ELSEIF },
	{ ".endif",		0,	0,	&DirectiveConditional,	DIRECTIVE_ENDIF },
	{ ".ifdef",		1,	1,	&DirectiveConditional,	DIRECTIVE_IFDEF },
	{ ".ifndef",	1,	1,	&DirectiveConditional,	DIRECTIVE_IFNDEF },
	{ ".elseifdef",	1,	1,	&DirectiveConditional,	DIRECTIVE_ELSEIFDEF },
	{ ".elseifndef",1,	1,	&DirectiveConditional,	DIRECTIVE_ELSEIFNDEF },
	{ ".ifarm",		0,	0,	&DirectiveConditional,	DIRECTIVE_IFARM },
	{ ".ifthumb",	0,	0,	&DirectiveConditional,	DIRECTIVE_IFTHUMB },

	{ ".area",		1,	1,	&DirectiveArea,			DIRECTIVE_AREASTART },
	{ ".endarea",	0,	0,	&DirectiveArea,			DIRECTIVE_AREAEND },

	{ ".include",	1,	1,	&DirectiveInclude,	0 },
	{ ".radix",		1,	1,	&DirectiveRadix,	0 },
	{ ".loadtable",	1,	1,	&DirectiveLoadTable,0 },
	{ ".table",		1,	1,	&DirectiveLoadTable,0 },
	{ ".string",	1,	64,	&DirectiveString,	0 },
	{ ".str",		1,	64,	&DirectiveString,	0 },
	{ ".psx",		0,	0,	&DirectivePsx,		0 },
	{ ".ps2",		0,	0,	&DirectivePs2,		DIRECTIVE_DISABLED },
	{ ".psp",		0,	0,	&DirectivePsp,		DIRECTIVE_DISABLED },
	{ ".gba",		0,	0,	&DirectiveGba,		0 },
	{ ".nds",		0,	0,	&DirectiveNds,		0 },
	{ ".gb",		0,	0,	&DirectiveGb,		DIRECTIVE_DISABLED },
	{ ".nocash",	0,	1,	&DirectiveNocash,	0 },
	{ ".definelabel",2,	2,	&DirectiveDefineLabel,	0 },
	{ ".relativeinclude",	1,	1,	&DirectiveRelativeInclude,	0 },
	{ ".erroronwarning",	1,	1,	&DirectiveWarningAsError,	0 },
	{ ".sym",		1,	1,	&DirectiveSym,	0 },

	{ ".warning",	1,	1,	&DirectiveMessage,	DIRECTIVE_WARNING },
	{ ".error",		1,	1,	&DirectiveMessage,	DIRECTIVE_ERROR },
	{ ".notice",	1,	1,	&DirectiveMessage,	DIRECTIVE_NOTICE },

	
	{ NULL,			0,	0,	NULL,					0 }
};

bool DirectiveAssembleGlobal(char* Name, char* Arguments)
{
	return DirectiveAssemble(Directives,Name,Arguments);
}