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

bool DirectiveOpen(ArgumentList& list, int flags)
{
	if (list.size() == 2)
	{
		// open
		CDirectiveFile* command = new CDirectiveFile(CDirectiveFile::Open,list);
		AddAssemblerCommand(command);
	} else {
		// copy
		CDirectiveFile* command = new CDirectiveFile(CDirectiveFile::Copy,list);
		AddAssemblerCommand(command);
	}
	return true;
}

bool DirectiveCreate(ArgumentList& list, int flags)
{
	CDirectiveFile* command = new CDirectiveFile(CDirectiveFile::Create,list);
	AddAssemblerCommand(command);
	return true;
}

bool DirectiveClose(ArgumentList& list, int flags)
{
	CDirectiveFile* command = new CDirectiveFile(CDirectiveFile::Close,list);
	AddAssemblerCommand(command);
	return true;
}

bool DirectiveOrg(ArgumentList& list, int flags)
{
	CDirectivePosition* command = new CDirectivePosition(CDirectivePosition::Virtual,list);
	AddAssemblerCommand(command);
	return true;
}

bool DirectiveOrga(ArgumentList& list, int flags)
{
	CDirectivePosition* command = new CDirectivePosition(CDirectivePosition::Physical,list);
	AddAssemblerCommand(command);
	return true;
}

bool DirectiveIncbin(ArgumentList& list, int flags)
{
	CDirectiveIncbin* command = new CDirectiveIncbin(list);
	AddAssemblerCommand(command);
	return true;
}

bool DirectiveAlign(ArgumentList& list, int flags)
{
	CDirectiveAlign* command = new CDirectiveAlign(list);
	AddAssemblerCommand(command);
	return true;
}

bool DirectiveHeaderSize(ArgumentList& list, int flags)
{
	CDirectiveHeaderSize* command = new CDirectiveHeaderSize(list);
	AddAssemblerCommand(command);
	return true;
}

bool DirectiveInclude(ArgumentList& List, int flags)
{
	std::wstring fileName = getFullPathName(List[0].text);

	TextFile::Encoding encoding = TextFile::GUESS;
	if (List.size() == 2)
		encoding = getEncodingFromString(List[1].text);

	int FileNum = Global.FileInfo.FileNum;
	int LineNum = Global.FileInfo.LineNumber;
	if (fileExists(fileName) == false)
	{
		Logger::printError(Logger::Error,L"Included file \"%s\" does not exist",fileName.c_str());
		return false;
	}
	LoadAssemblyFile(fileName,encoding);
	Global.FileInfo.FileNum = FileNum;
	Global.FileInfo.LineNumber = LineNum;
	return true;
}

bool DirectiveData(ArgumentList& List, int flags)
{
	bool ascii = false;

	if (flags & DIRECTIVE_DATA_ASCII)
	{
		ascii = true;
		flags &= ~DIRECTIVE_DATA_ASCII;
	}

	bool hasNonAscii = false;
	for (size_t i = 0; i < List.size(); i++)
	{
		if (List[i].isString)
		{
			for (size_t k = 0; k < List[i].text.size(); k++)
			{
				if (List[i].text[k] >= 0x80)
					hasNonAscii = true;
			}
		}
	}

	if (hasNonAscii)
		Logger::printError(Logger::Warning,L"Non-ASCII character in data directive. Use .string instead");

	CDirectiveData* Data = new CDirectiveData(List,flags,ascii);
	AddAssemblerCommand(Data);
	return true;
}

bool DirectiveRadix(ArgumentList& List, int flags)
{
	int rad;
	if (ConvertExpression(List[0].text,rad) == false)
	{
		Logger::printError(Logger::Error,L"Invalid expression %s",List[0].text.c_str());
		return false;
	}

	switch (rad)
	{
	case 2: case 8: case 10: case 16:
		Global.Radix = rad;
		break;
	default:
		Logger::printError(Logger::Error,L"Invalid radix %d",rad);
		return false;;
	}
	return true;
}

bool DirectiveLoadTable(ArgumentList& List, int flags)
{
	std::wstring fileName = getFullPathName(List[0].text);

	if (fileExists(fileName) == false)
	{
		Logger::printError(Logger::Error,L"Table file \"%s\" does not exist",fileName.c_str());
		return false;
	}

	TextFile::Encoding encoding = TextFile::GUESS;
	if (List.size() == 2)
		encoding = getEncodingFromString(List[1].text);

	if (Global.Table.load(fileName,encoding) == false)
	{
		Logger::printError(Logger::Error,L"Invalid table file \"%s\"",fileName.c_str());
		return false;
	}
	return true;
}

bool DirectiveString(ArgumentList& List, int flags)
{
	ArgumentList NewList;

	if (Global.Table.isLoaded() == false)
	{
		Logger::printError(Logger::Error,L"No table opened");
		return false;
	}

	for (size_t i = 0; i < List.size(); i++)
	{
		if (List[i].isString)
		{
			ByteArray data = Global.Table.encodeString(List[i].text,false);

			if (data.size() == 0 && List[i].text.size() != 0)
			{
				Logger::printError(Logger::Error,L"Failed to encode string");
				return false;
			}

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

	if ((flags & DIRECTIVE_STR_NOTERMINATION) == 0)
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

bool DirectiveSJIS(ArgumentList& List, int flags)
{
	ArgumentList NewList;
	
	if (Global.SJISTable.isLoaded() == false) Global.SJISTable.generateSJIS();

	for (size_t i = 0; i < List.size(); i++)
	{
		if (List[i].isString)
		{
			ByteArray data = Global.SJISTable.encodeString(List[i].text,false);

			if (data.size() == 0 && List[i].text.size() != 0)
			{
				Logger::printError(Logger::Error,L"Failed to encode string");
				return false;
			}

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

	if ((flags & DIRECTIVE_STR_NOTERMINATION) == 0)
	{
		ByteArray data = Global.SJISTable.encodeTermination();
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
			Logger::printError(Logger::Error,L"Invalid arguments");
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
		Logger::printError(Logger::Error,L"Invalid expression \"%s\"",List[1].text.c_str());
		return false;
	}
	
	Label* label = Global.symbolTable.getLabel(List[0].text,Global.FileInfo.FileNum,Global.Section);
	if (label == NULL)
	{
		Logger::printError(Logger::Error,L"Invalid label name \"%s\"",List[0].text.c_str());
		return false;
	}

	if (label->isDefined())
	{
		Logger::printError(Logger::Error,L"Label \"%s\" already defined",List[0].text.c_str());
		return false;
	}
	
	labelCommand = new CAssemblerLabel(label->getName(),value,Global.Section,true);
	AddAssemblerCommand(labelCommand);
	return true;
}

bool DirectiveConditional(ArgumentList& List, int flags)
{
	int value;

	switch (flags)
	{
	case DIRECTIVE_COND_IF:			value = CONDITIONAL_IF; break;
	case DIRECTIVE_COND_ELSE:		value = CONDITIONAL_ELSE; break;
	case DIRECTIVE_COND_ELSEIF:		value = CONDITIONAL_ELSEIF; break;
	case DIRECTIVE_COND_ENDIF:		value = CONDITIONAL_ENDIF; break;
	case DIRECTIVE_COND_IFDEF:		value = CONDITIONAL_IFDEF; break;
	case DIRECTIVE_COND_IFNDEF:		value = CONDITIONAL_IFNDEF; break;
	case DIRECTIVE_COND_ELSEIFDEF:	value = CONDITIONAL_ELSEIFDEF; break;
	case DIRECTIVE_COND_ELSEIFNDEF:	value = CONDITIONAL_ELSEIFNDEF; break;
	case DIRECTIVE_COND_IFARM:		value = CONDITIONAL_IFARM; break;
	case DIRECTIVE_COND_IFTHUMB:	value = CONDITIONAL_IFTHUMB; break;
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

	switch (flags)
	{
	case DIRECTIVE_AREA_START:
		Command = new CDirectiveArea();
		if (Command->LoadStart(List) == false)
		{
			delete Command;
			return false;
		}
		break;
	case DIRECTIVE_AREA_END:
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
	CDirectiveMessage::Type value;

	switch (flags)
	{
	case DIRECTIVE_MSG_WARNING:		value = CDirectiveMessage::Warning;	break;
	case DIRECTIVE_MSG_ERROR:		value = CDirectiveMessage::Error;	break;
	case DIRECTIVE_MSG_NOTICE:		value = CDirectiveMessage::Notice;	break;
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
			Logger::printError(Logger::Error,L"Invalid arguments");
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
			Logger::printError(Logger::Error,L"Invalid arguments");
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
		Logger::printError(Logger::Error,L"Invalid arguments");
		return false;
	}
	return true;
}

bool DirectiveStartFunction(ArgumentList& List, int flags)
{
	CDirectiveFunction* func = new CDirectiveFunction(List[0].text,Global.Section);
	AddAssemblerCommand(func);
	return true;
}

bool DirectiveEndFunction(ArgumentList& List, int flags)
{
	CDirectiveFunction* func = new CDirectiveFunction(L"",Global.Section);
	AddAssemblerCommand(func);
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
				Logger::printError(Logger::Error,L"Parameter failure (empty argument)");
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
					Logger::printError(Logger::Error,L"Unexpected end of line in string");
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
			Logger::printError(Logger::Error,L"parameter replacement length overflow");
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

	if (directive.minparam > (short)arguments.size())
	{
		Logger::printError(Logger::Error,L"Not enough parameters (min %d)",
			directive.minparam);
		return false;
	}

	if (directive.maxparam != -1 && (directive.maxparam < (short)arguments.size()))
	{
		Logger::printError(Logger::Error,L"Too many parameters (max %d)",
			directive.maxparam);
		return false;
	}

	if (directive.Flags & DIRECTIVE_MIPSRESETDELAY)
	{
		Arch->NextSection();
	}

	return directive.Function(arguments,directive.Flags & DIRECTIVE_USERMASK);
}

int getDirective(const tDirective* DirectiveSet, const std::wstring& name)
{
	int num = -1;
	for (int z = 0; DirectiveSet[z].name != NULL; z++)
	{
		if (DirectiveSet[z].Flags & DIRECTIVE_DISABLED) continue;
		if (DirectiveSet[z].Flags & DIRECTIVE_NOCASHOFF && Global.nocash == true) continue;
		if (DirectiveSet[z].Flags & DIRECTIVE_NOCASHON && Global.nocash == false) continue;
		if (wcscmp(name.c_str(),DirectiveSet[z].name) == 0)
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
	{ L".open",				2,	3,	&DirectiveOpen,				0 },
	{ L".openfile",			2,	3,	&DirectiveOpen,				0 },
	{ L".create",			2,	2,	&DirectiveCreate,			0 },
	{ L".createfile",		2,	2,	&DirectiveCreate,			0 },
	{ L".close",			0,	0,	&DirectiveClose,			0 },
	{ L".closefile",		0,	0,	&DirectiveClose,			0 },
	{ L".incbin",			1,	3,	&DirectiveIncbin,			0 },
	{ L".import",			1,	3,	&DirectiveIncbin,			0 },
	{ L".org",				1,	1,	&DirectiveOrg,				0 },
	{ L"org",				1,	1,	&DirectiveOrg,				0 },
	{ L".orga",				1,	1,	&DirectiveOrga,				0 },
	{ L"orga",				1,	1,	&DirectiveOrga,				0 },
	{ L".align",			0,	1,	&DirectiveAlign,			0 },
	{ L".headersize",		1,	1,	&DirectiveHeaderSize,		0 },

	{ L".fill",				1,	2,	&DirectiveFill,				0 },
	{ L"defs",				1,	2,	&DirectiveFill,				0 },

	{ L".byte",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_8 },
	{ L".halfword",			1,	-1,	&DirectiveData,				DIRECTIVE_DATA_16 },
	{ L".word",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_32 },
	{ L".db",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_8 },
	{ L".dh",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_16|DIRECTIVE_NOCASHOFF },
	{ L".dw",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_32|DIRECTIVE_NOCASHOFF },
	{ L".dw",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_16|DIRECTIVE_NOCASHON },
	{ L".dd",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_32|DIRECTIVE_NOCASHON },
	{ L".dcb",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_8 },
	{ L".dcw",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_16 },
	{ L".dcd",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_32 },
	{ L"db",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_8 },
	{ L"dh",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_16|DIRECTIVE_NOCASHOFF },
	{ L"dw",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_32|DIRECTIVE_NOCASHOFF },
	{ L"dw",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_16|DIRECTIVE_NOCASHON },
	{ L"dd",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_32|DIRECTIVE_NOCASHON },
	{ L"dcb",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_8 },
	{ L"dcw",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_16 },
	{ L"dcd",				1,	-1,	&DirectiveData,				DIRECTIVE_DATA_32 },
	{ L".ascii",			1,	-1,	&DirectiveData,				DIRECTIVE_DATA_8|DIRECTIVE_DATA_ASCII },

	{ L".if",				1,	1,	&DirectiveConditional,		DIRECTIVE_COND_IF },
	{ L".else",				0,	0,	&DirectiveConditional,		DIRECTIVE_COND_ELSE },
	{ L".elseif",			1,	1,	&DirectiveConditional,		DIRECTIVE_COND_ELSEIF },
	{ L".endif",			0,	0,	&DirectiveConditional,		DIRECTIVE_COND_ENDIF },
	{ L".ifdef",			1,	1,	&DirectiveConditional,		DIRECTIVE_COND_IFDEF },
	{ L".ifndef",			1,	1,	&DirectiveConditional,		DIRECTIVE_COND_IFNDEF },
	{ L".elseifdef",		1,	1,	&DirectiveConditional,		DIRECTIVE_COND_ELSEIFDEF },
	{ L".elseifndef",		1,	1,	&DirectiveConditional,		DIRECTIVE_COND_ELSEIFNDEF },
	{ L".ifarm",			0,	0,	&DirectiveConditional,		DIRECTIVE_COND_IFARM },
	{ L".ifthumb",			0,	0,	&DirectiveConditional,		DIRECTIVE_COND_IFTHUMB },

	{ L".area",				1,	1,	&DirectiveArea,				DIRECTIVE_AREA_START },
	{ L".endarea",			0,	0,	&DirectiveArea,				DIRECTIVE_AREA_END },

	{ L".include",			1,	2,	&DirectiveInclude,			0 },
	{ L".radix",			1,	1,	&DirectiveRadix,			0 },
	{ L".loadtable",		1,	2,	&DirectiveLoadTable,		0 },
	{ L".table",			1,	2,	&DirectiveLoadTable,		0 },
	{ L".string",			1,	-1,	&DirectiveString,			0 },
	{ L".str",				1,	-1,	&DirectiveString,			0 },
	{ L".stringn",			1,	-1,	&DirectiveString,			DIRECTIVE_STR_NOTERMINATION },
	{ L".strn",				1,	-1,	&DirectiveString,			DIRECTIVE_STR_NOTERMINATION },
	{ L".sjis",				1,	-1,	&DirectiveSJIS,				0 },
	{ L".sjisn",			1,	-1,	&DirectiveSJIS,				DIRECTIVE_STR_NOTERMINATION },
	{ L".psx",				0,	0,	&DirectivePsx,				0 },
	{ L".ps2",				0,	0,	&DirectivePs2,				DIRECTIVE_DISABLED },
	{ L".psp",				0,	0,	&DirectivePsp,				0 },
	{ L".gba",				0,	0,	&DirectiveGba,				0 },
	{ L".nds",				0,	0,	&DirectiveNds,				0 },
	{ L".gb",				0,	0,	&DirectiveGb,				DIRECTIVE_DISABLED },
	{ L".nocash",			0,	1,	&DirectiveNocash,			0 },
	{ L".definelabel",		2,	2,	&DirectiveDefineLabel,		0 },
	{ L".relativeinclude",	1,	1,	&DirectiveRelativeInclude,	0 },
	{ L".erroronwarning",	1,	1,	&DirectiveWarningAsError,	0 },
	{ L".sym",				1,	1,	&DirectiveSym,				0 },

	{ L".function",			1,	1,	&DirectiveStartFunction,	0 },
	{ L".func",				1,	1,	&DirectiveStartFunction,	0 },
	{ L".endfunction",		0,	0,	&DirectiveEndFunction,		0 },
	{ L".endfunc",			0,	0,	&DirectiveEndFunction,		0 },

	{ L".warning",			1,	1,	&DirectiveMessage,			DIRECTIVE_MSG_WARNING },
	{ L".error",			1,	1,	&DirectiveMessage,			DIRECTIVE_MSG_ERROR },
	{ L".notice",			1,	1,	&DirectiveMessage,			DIRECTIVE_MSG_NOTICE },

	{ NULL,					0,	0,	NULL,						0 }
};

bool directiveAssembleGlobal(const std::wstring& name, const std::wstring& arguments)
{
	return directiveAssemble(Directives,name,arguments);
}