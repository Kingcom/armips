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
	Global.Error = false;
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

	printf("ARMIPS Assembler v0.7d ("__DATE__" "__TIME__") by Kingcom\n");
	StringList arguments = getStringListFromArray(argv,argc);

	if (arguments.size() < 2)
	{
		printf("Usage: armips.exe file.asm [-temp temp.txt] [-sym symfile.sym]\n");
		return 1;
	}

	if (fileExists(arguments[1]) == false)
	{
		printf("File %s not found\n",argv[1]);
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
			printf("Invalid parameter %ls\n",arguments[argpos].c_str());
			return 1;
		}
	}

	LoadAssemblyFile(arguments[1]);
	if (Global.Error == true)
	{
		printf("Aborting.\n");
		return 1;
	}
	if (EncodeAssembly() == true)
	{
		printf("Done.\n");
	} else {
		printf("Aborting.\n");
		return 1;
	}
	return 0;
}

