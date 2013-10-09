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

bool DirectiveFile(ArgumentList& List, int flags)
{
	eDirectiveFileMode value;

	switch (flags & DIRECTIVE_PARAMFIELD)
	{
	case DIRECTIVE_OPEN:
		if (List.size() == 3) value = DIRECTIVEFILE_COPY; else value = DIRECTIVEFILE_OPEN;
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

bool DirectiveInclude(ArgumentList& List, int flags)
{
	std::wstring fileName = getFullPathName(List[0].text);

	int FileNum = Global.FileInfo.FileNum;
	int LineNum = Global.FileInfo.LineNumber;
	if (fileExists(fileName) == false)
	{
		PrintError(ERROR_ERROR,"Included file \"%ls\" does not exist",fileName.c_str());
		return false;
	}
	LoadAssemblyFile(fileName);
	Global.FileInfo.FileNum = FileNum;
	Global.FileInfo.LineNumber = LineNum;
	return true;
}

bool DirectiveData(ArgumentList& List, int flags)
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

bool DirectiveRadix(ArgumentList& List, int flags)
{
	int rad;
	if (ConvertExpression(List[0].text,rad) == false)
	{
		PrintError(ERROR_ERROR,"Invalid expression %ls",List[0].text.c_str());
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

bool DirectiveLoadTable(ArgumentList& List, int flags)
{
	std::wstring fileName = getFullPathName(List[0].text);

	if (fileExists(fileName) == false)
	{
		PrintError(ERROR_ERROR,"Table file \"%ls\" does not exist",fileName.c_str());
		return false;
	}
	if (Global.Table.load(fileName) == false)
	{
		PrintError(ERROR_ERROR,"Invalid table file \"%ls\"",fileName.c_str());
		return false;
	}
	return true;
}

bool DirectiveString(ArgumentList& List, int flags)
{
	ArgumentList NewList;

	if (Global.Table.isLoaded() == false)
	{
		PrintError(ERROR_ERROR,"No table opened");
		return false;
	}

	for (size_t i = 0; i < List.size(); i++)
	{
		if (List[i].isString)
		{
			ByteArray data = Global.Table.encodeString(List[i].text,false);

			for (int i = 0; i < data.size(); i++)
			{
				wchar_t str[32];
				swprintf(str,32,L"0x%02X",data[i]);
				NewList.add(str,false);
			}
		} else {
			NewList.add(List[i].text,false);
		}
	}

	if ((flags & DIRECTIVE_NOTERMINATION) == 0)
	{
		ByteArray data = Global.Table.encodeTermination();
		for (int i = 0; i < data.size(); i++)
		{
			wchar_t str[32];
			swprintf(str,32,L"0x%02X",data[i]);
			NewList.add(str,false);
		}
	}

	CDirectiveData* Data = new CDirectiveData(NewList,1,false);
	AddAssemblerCommand(Data);
	return true;
}

bool DirectivePsx(ArgumentList& List, int flags)
{
	Arch = &Mips;
	Mips.SetLoadDelay(false,0);
	Mips.SetVersion(MARCH_PSX);
	return true;
}

bool DirectivePs2(ArgumentList& List, int flags)
{
	Arch = &Mips;
	Mips.SetLoadDelay(false,0);
	Mips.SetVersion(MARCH_PS2);
	return true;
}

bool DirectivePsp(ArgumentList& List, int flags)
{
	Arch = &Mips;
	Mips.SetLoadDelay(false,0);
	Mips.SetVersion(MARCH_PSP);
	return true;
}

bool DirectiveGba(ArgumentList& List, int flags)
{
	Arch = &Arm;
	Arm.SetThumbMode(true);
	Arm.SetArm9(false);
	
	ArmStateCommand* cmd = new ArmStateCommand(false);
	AddAssemblerCommand(cmd);
	return true;
}

bool DirectiveNds(ArgumentList& List, int flags)
{
	Arch = &Arm;
	Arm.SetThumbMode(false);
	Arm.SetArm9(true);

	ArmStateCommand* cmd = new ArmStateCommand(true);
	AddAssemblerCommand(cmd);
	return true;
}

bool DirectiveGb(ArgumentList& List, int flags)
{
	Arch = &z80;
	return true;
}

bool DirectiveNocash(ArgumentList& List, int flags)
{
	if (List.size() == 1)
	{
		if (List[0].text == L"on")
		{
			Global.nocash = true;
		} else if (List[0].text == L"off")
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

bool DirectiveFill(ArgumentList& List, int flags)
{
	CDirectiveFill* Command = new CDirectiveFill();
	if (Command->Load(List) == false) return false;
	AddAssemblerCommand(Command);
	return true;
}

bool DirectiveDefineLabel(ArgumentList& List, int flags)
{
	int value;
	CAssemblerLabel* labelCommand;

	if (ConvertExpression(List[1].text,value) == false)
	{
		PrintError(ERROR_ERROR,"Invalid expression \"%ls\"",List[1].text.c_str());
		return false;
	}
	
	Label* label = Global.symbolTable.getLabel(List[0].text,Global.FileInfo.FileNum,Global.Section);
	if (label == NULL)
	{
		PrintError(ERROR_ERROR,"Invalid label name \"%ls\"",List[0].text.c_str());
		return false;
	}

	if (label->isDefined())
	{
		PrintError(ERROR_ERROR,"Label \"%ls\" already defined",List[0].text.c_str());
		return false;
	}
	
	labelCommand = new CAssemblerLabel(label->getName(),value,Global.Section,true);
	AddAssemblerCommand(labelCommand);
	return true;
}

bool DirectiveConditional(ArgumentList& List, int flags)
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

bool DirectiveArea(ArgumentList& List, int flags)
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


bool DirectiveMessage(ArgumentList& List, int flags)
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


bool DirectiveWarningAsError(ArgumentList& List, int flags)
{
	if (List.size() == 1)
	{
		if (List[0].text == L"on")
		{
			Global.warningAsError = true;
		} else if (List[0].text == L"off")
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

bool DirectiveRelativeInclude(ArgumentList& List, int flags)
{
	if (List.size() == 1)
	{
		if (List[0].text == L"on")
		{
			Global.relativeInclude = true;
		} else if (List[0].text == L"off")
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

bool DirectiveSym(ArgumentList& List, int flags)
{
	if (List[0].text == L"on")
	{
		CDirectiveSym* sym = new CDirectiveSym(true);
		AddAssemblerCommand(sym);
	} else if (List[0].text == L"off")
	{
		CDirectiveSym* sym = new CDirectiveSym(false);
		AddAssemblerCommand(sym);
	} else {
		PrintError(ERROR_ERROR,"Invalid arguments");
		return false;
	}
	return true;
}

bool splitArguments(ArgumentList& list, const std::wstring& args)
{
	std::wstring buffer;
	size_t pos = 0;
	bool isString = false;

	list.clear();

	while (pos < args.size())
	{
		while (pos < args.size() && (args[pos] == ' ' || args[pos] == '\t')) pos++;
		if (pos == args.size()) break;

		if (args[pos] == ',')
		{
			if (buffer.empty())
			{
				PrintError(ERROR_ERROR,"Parameter failure (empty argument)");
				return false;
			}

			list.add(buffer,isString);
			buffer.clear();
			isString = false;
			pos++;
			continue;
		}

		if (args[pos] == '"')
		{
			pos++;
			while (args[pos] != '"')
			{
				if (pos == args.size())
				{
					PrintError(ERROR_ERROR,"Unexpected end of line in string");
					return false;
				}

				if (args[pos] == '\\' && pos+1 < args.size())
				{
					if (args[pos+1] == '\\')
					{
						buffer += '\\';
						pos += 2;
						continue;
					}
					
					if (args[pos+1] == '"')
					{
						buffer += '"';
						pos += 2;
						continue;
					}
				}

				buffer += args[pos++];
			}
			
			isString = true;
			pos++;
			continue;
		}

		buffer += args[pos++];
		if (buffer.size() >= 2048)
		{
			PrintError(ERROR_ERROR,"parameter replacement length overflow");
			return false;
		}
	}

	if (buffer.empty() == false || isString == true)
		list.add(buffer,isString);

	return true;
}

bool executeDirective(const tDirective& directive, const std::wstring& args)
{
	ArgumentList arguments;
	if (splitArguments(arguments,args) == false) return false;

	if (directive.minparam > arguments.size())
	{
		PrintError(ERROR_ERROR,"Not enough parameters (min %d)",
			directive.minparam);
		return false;
	}

	if (directive.maxparam < arguments.size())
	{
		PrintError(ERROR_ERROR,"Too many parameters (max %d)",
			directive.maxparam);
		return false;
	}

	if (directive.Flags & DIRECTIVE_MIPSRESETDELAY)
	{
		Arch->NextSection();
	}

	return directive.Function(arguments,directive.Flags);
}

int getDirective(const tDirective* DirectiveSet, const std::wstring& name)
{
	std::string utf8 = convertWStringToUtf8(name);

	int num = -1;
	for (int z = 0; DirectiveSet[z].name != NULL; z++)
	{
		if (DirectiveSet[z].Flags & DIRECTIVE_DISABLED) continue;
		if (DirectiveSet[z].Flags & DIRECTIVE_NOCASHOFF && Global.nocash == true) continue;
		if (DirectiveSet[z].Flags & DIRECTIVE_NOCASHON && Global.nocash == false) continue;
		if (strcmp(utf8.c_str(),DirectiveSet[z].name) == 0)
		{
			num = z;
			break;
		}
	}
	return num;
}

bool directiveAssemble(const tDirective* directiveSet, const std::wstring& name, const std::wstring& arguments)
{
	int num = getDirective(directiveSet,name);
	if (num == -1) return false;
	executeDirective(directiveSet[num],arguments);
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
	{ ".stringn",	1,	64,	&DirectiveString,	DIRECTIVE_NOTERMINATION },
	{ ".strn",		1,	64,	&DirectiveString,	DIRECTIVE_NOTERMINATION },
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

bool directiveAssembleGlobal(const std::wstring& name, const std::wstring& arguments)
{
	return directiveAssemble(Directives,name,arguments);
}