#include "stdafx.h"
#include "Core/Common.h"
#include "Core/MathParser.h"
#include "Util/CommonClasses.h"
#include "Core/Assembler.h"
#include "Archs/MIPS/Mips.h"
#include "Commands/CDirectiveFile.h"

int wmain(int argc, wchar_t* argv[])
{
	Global.Radix = 10;
	Global.FileOpened = false;
	Global.HeaderSize = 0;
	Global.Quit = false;
	Global.RamPos = 0;
	Global.Revalidate = true;
	Global.Section = 0;
	Global.nocash = false;
	Global.IncludeNestingLevel = 0;
	Global.FileInfo.FileCount = 0;
	Global.FileInfo.TotalLineCount = 0;
	Global.DebugMessages = 0;
	Global.relativeInclude = false;
	Global.warningAsError = false;
	Global.validationPasses = 0;
	Arch = &InvalidArchitecture;

	Logger::printLine("ARMIPS Assembler v0.7d ("__DATE__" "__TIME__") by Kingcom");
	StringList arguments = getStringListFromArray(argv,argc);

	if (arguments.size() < 2)
	{
		Logger::printLine("Usage: armips.exe file.asm [-temp temp.txt] [-sym symfile.sym]");
		return 1;
	}

	if (fileExists(arguments[1]) == false)
	{
		Logger::printLine("File %s not found\n",arguments[1].c_str());
		return 1;
	}

	int argpos = 2;
	while (argpos < arguments.size())
	{
		if (arguments[argpos] == L"-temp")
		{
			Global.tempData.setFileName(arguments[argpos+1]);
			argpos += 2;
		} else if (arguments[argpos] == L"-sym")
		{
			Global.symData.setFileName(arguments[argpos+1]);
			argpos += 2;
		} else if (arguments[argpos] == L"-erroronwarning")
		{
			Global.warningAsError = true;
			argpos += 1;
		} else {
			Logger::printLine("Invalid parameter %s\n",arguments[argpos].c_str());
			return 1;
		}
	}

	LoadAssemblyFile(arguments[1]);
	if (Logger::hasError())
	{
		Logger::printLine("Aborting.");
		return 1;
	}
	if (EncodeAssembly() == true)
	{
		Logger::printLine("Done.");
	} else {
		Logger::printLine("Aborting.");
		return 1;
	}
	return 0;
}

