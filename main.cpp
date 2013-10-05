#include "stdafx.h"
#include "Core/Common.h"
#include "Core/MathParser.h"
#include "Util/CommonClasses.h"
#include "Core/Assembler.h"
#include "Archs/MIPS/Mips.h"
#include "Commands/CDirectiveFile.h"

int _tmain(int argc, _TCHAR* argv[])
{
	Global.Radix = 10;
	Global.Error = false;
	Global.FileOpened = false;
	Global.HeaderSize = 0;
	Global.Quit = false;
	Global.RamPos = 0;
	Global.Revalidate = true;
	Global.Section = 0;
	Global.SymData.Write = false;
	Global.TempData.Write = false;
	Global.nocash = false;
	Global.ConditionData.EntryCount = 0;
	Global.AreaData.EntryCount = 0;
	Global.IncludeNestingLevel = 0;
	Global.FileInfo.FileCount = 0;
	Global.FileInfo.TotalLineCount = 0;
	Global.DebugMessages = 0;
	Global.relativeInclude = false;
	Global.warningAsError = false;
	Global.validationPasses = 0;
	Arch = &InvalidArchitecture;

	printf("ARMIPS Assembler v0.7d ("__DATE__" "__TIME__") by Kingcom\n");

	if (argc < 2)
	{
		printf("Usage: armips.exe file.asm [-temp temp.txt] [-sym symfile.sym]\n");
		return 1;
	}

	if (FileExists(argv[1]) == false)
	{
		printf("File %s not found\n",argv[1]);
		return 1;
	}

	int argpos = 2;
	while (argpos < argc)
	{
		if (strcmp(argv[argpos],"-temp") == 0)
		{
			strcpy(Global.TempData.Name,argv[argpos+1]);
			Global.TempData.Write = true;
			argpos += 2;
		} else if (strcmp(argv[argpos],"-sym") == 0)
		{
			strcpy(Global.SymData.Name,argv[argpos+1]);
			Global.SymData.Write = true;
			argpos += 2;
		} else if (strcmp(argv[argpos],"-erroronwarning") == 0)
		{
			Global.warningAsError = true;
			argpos += 1;
		} else {
			printf("Invalid parameter %s\n",argv[argpos]);
			return 1;
		}
	}

	LoadAssemblyFile(argv[1]);
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

