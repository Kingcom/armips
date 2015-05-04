#include "stdafx.h"
#include "Core/Directives.h"
#include "Core/Common.h"
#include "Commands/CDirectiveFile.h"
#include "Commands/CDirectiveData.h"
#include "Commands/CDirectiveConditional.h"
#include "Commands/CDirectiveMessage.h"
#include "Commands/CDirectiveArea.h"
#include "Commands/CAssemblerLabel.h"
#include "Assembler.h"
#include "Archs/MIPS/Mips.h"
#include "Archs/ARM/Arm.h"
#include "Core/Expression.h"
#include "Util/Util.h"

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
		Logger::printError(Logger::Error,L"Included file \"%s\" does not exist",fileName);
		return false;
	}
	LoadAssemblyFile(fileName,encoding);
	Global.FileInfo.FileNum = FileNum;
	Global.FileInfo.LineNumber = LineNum;
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

bool DirectiveMessage(ArgumentList& List, int flags)
{
	CDirectiveMessage::Type value;

	switch (flags)
	{
	case DIRECTIVE_MSG_WARNING:		value = CDirectiveMessage::Type::Warning;	break;
	case DIRECTIVE_MSG_ERROR:		value = CDirectiveMessage::Type::Error;		break;
	case DIRECTIVE_MSG_NOTICE:		value = CDirectiveMessage::Type::Notice;	break;
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
			Logger::setErrorOnWarning(true);
		} else if (List[0].text == L"off")
		{
			Logger::setErrorOnWarning(false);
		} else {
			Logger::printError(Logger::Error,L"Invalid arguments");
			return false;
		}
	} else {
		Logger::setErrorOnWarning(true);
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

bool DirectiveImportObj(ArgumentList& list, int flags)
{
	DirectiveObjImport* command = new DirectiveObjImport(list);
	AddAssemblerCommand(command);
	return true;
}


size_t loadArgument(ArgumentList& list, const std::wstring& args, size_t pos)
{
	std::wstring buffer;

	if (args[pos] == '"')
	{
		pos++;
		while (pos < args.size() && args[pos] != '"')
		{
			if (args[pos] == '\\' && pos+1 < args.size())
			{
				buffer += '\\';

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
			
		if (pos == args.size() || args[pos] != '"')
		{
			Logger::printError(Logger::Error,L"Unexpected end of line in string");
			return false;
		}

		list.add(buffer,true);
		pos++;
	} else {
		while (pos < args.size() && args[pos] != ',')
		{
			if (args[pos] == ' ' || args[pos] == '\t')
			{
				pos++;
				continue;
			}
			buffer += args[pos++];
		}
		
		list.add(buffer,false);
	}

	return pos;
}

bool splitArguments(ArgumentList& list, const std::wstring& args)
{
	std::wstring buffer;
	size_t pos = 0;

	list.clear();

	while (pos < args.size())
	{
		while (pos < args.size() && (args[pos] == ' ' || args[pos] == '\t'))
			pos++;

		if (pos == args.size())
			break;

		if (list.size() != 0 && args[pos++] != ',')
		{
			Logger::printError(Logger::Error,L"Parameter failure");
			return false;
		}
		
		if (args[pos] == ',')
		{
			Logger::printError(Logger::Error,L"Parameter failure (empty argument)");
			return false;
		}

		while (pos < args.size() && (args[pos] == ' ' || args[pos] == '\t'))
			pos++;

		if (pos == args.size())
		{
			Logger::printError(Logger::Error,L"Parameter failure (empty argument)");
			return false;
		}

		pos = loadArgument(list,args,pos);
	}

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
		if (DirectiveSet[z].Flags & DIRECTIVE_NOTINMEMORY && Global.memoryMode == true) continue;
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
	{ L".include",			1,	2,	&DirectiveInclude,			0 },
	{ L".nocash",			0,	1,	&DirectiveNocash,			0 },
	{ L".relativeinclude",	1,	1,	&DirectiveRelativeInclude,	0 },
	{ L".erroronwarning",	1,	1,	&DirectiveWarningAsError,	0 },
	{ L".sym",				1,	1,	&DirectiveSym,				0 },
	{ L".importobj",		1,	2,	&DirectiveImportObj,		0 },
	{ L".importlib",		1,	2,	&DirectiveImportObj,		0 },

	{ L".warning",			1,	1,	&DirectiveMessage,			DIRECTIVE_MSG_WARNING },
	{ L".error",			1,	1,	&DirectiveMessage,			DIRECTIVE_MSG_ERROR },
	{ L".notice",			1,	1,	&DirectiveMessage,			DIRECTIVE_MSG_NOTICE },

	{ NULL,					0,	0,	NULL,						0 }
};

bool directiveAssembleGlobal(const std::wstring& name, const std::wstring& arguments)
{
	return directiveAssemble(Directives,name,arguments);
}