#include "stdafx.h"
#include "Core/Common.h"
#include "Core/MathParser.h"
#include "Util/CommonClasses.h"
#include "Core/Assembler.h"
#include "Archs/MIPS/Mips.h"
#include "Commands/CDirectiveFile.h"
#include "Core/Tests.h"

int wmain(int argc, wchar_t* argv[])
{
	AssemblerArguments parameters;

#ifdef ARMIPS_TESTS
	if (argc < 2)
	{
		Logger::printLine("Not enough arguments");
		return 1;
	}

	return !runTests(argv[1]);
#endif

	Logger::printLine("ARMIPS Assembler v0.7d ("__DATE__" "__TIME__") by Kingcom");
	StringList arguments = getStringListFromArray(argv,argc);

	if (arguments.size() < 2)
	{
		Logger::printLine("Usage: armips.exe file.asm [-temp temp.txt] [-sym symfile.sym]");
		return 1;
	}

	parameters.inputFileName = arguments[1];
	if (fileExists(parameters.inputFileName) == false)
	{
		Logger::printLine("File %S not found\n",parameters.inputFileName.c_str());
		return 1;
	}

	size_t argpos = 2;
	while (argpos < arguments.size())
	{
		if (arguments[argpos] == L"-temp")
		{
			parameters.tempFileName = arguments[argpos+1];
			argpos += 2;
		} else if (arguments[argpos] == L"-sym")
		{
			parameters.symFileName = arguments[argpos+1];
			parameters.symFileVersion = 1;
			argpos += 2;
		} else if (arguments[argpos] == L"-sym2")
		{
			parameters.symFileName = arguments[argpos+1];
			parameters.symFileVersion = 2;
			argpos += 2;
		} else if (arguments[argpos] == L"-exsym")
		{
			parameters.exSymFileName = arguments[argpos+1];
			argpos += 2;
		} else if (arguments[argpos] == L"-erroronwarning")
		{
			parameters.errorOnWarning = true;
			argpos += 1;
		} else {
			Logger::printLine("Invalid parameter %S\n",arguments[argpos].c_str());
			return 1;
		}
	}

	bool result = runAssembler(parameters);
	if (result == false)
	{
		Logger::printLine("Aborting.");
		return 1;
	}
	
	Logger::printLine("Done.");
	return 0;
}

