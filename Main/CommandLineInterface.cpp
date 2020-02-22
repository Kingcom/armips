#include "stdafx.h"
#include "Core/Common.h"
#include "Core/Assembler.h"
#include "CommandLineInterface.h"

#if defined(_WIN64) || defined(__x86_64__) || defined(__amd64__)
#define ARMIPSNAME "ARMIPS64"
#else
#define ARMIPSNAME "ARMIPS"
#endif

static void printUsage(std::wstring executableName)
{
	Logger::printLine(L"%s Assembler v%d.%d.%d (%s %s) by Kingcom",
		ARMIPSNAME, ARMIPS_VERSION_MAJOR, ARMIPS_VERSION_MINOR, ARMIPS_VERSION_REVISION, __DATE__, __TIME__);
	Logger::printLine(L"Usage: %s [optional parameters] <FILE>", executableName);
	Logger::printLine(L"");
	Logger::printLine(L"Optional parameters:");
	Logger::printLine(L" -temp <TEMP>              Output temporary assembly data to <TEMP> file");
	Logger::printLine(L" -sym  <SYM>               Output symbol data in the sym format to <SYM> file");
	Logger::printLine(L" -sym2 <SYM2>              Output symbol data in the sym2 format to <SYM2> file");
	Logger::printLine(L" -root <ROOT>              Use <ROOT> as working directory during execution");
	Logger::printLine(L" -equ  <NAME> <VAL>        Equivalent to \'<NAME> equ <VAL>\' in code");
	Logger::printLine(L" -strequ <NAME> <VAL>      Equivalent to \'<NAME> equ \"<VAL>\"\' in code");
	Logger::printLine(L" -definelabel <NAME> <VAL> Equivalent to \'.definelabel <NAME>, <VAL>\' in code");
	Logger::printLine(L" -erroronwarning           Treat all warnings like errors");
	Logger::printLine(L"");
	Logger::printLine(L"File arguments:");
	Logger::printLine(L" <FILE>                    Main assembly code file");
}

static bool parseArguments(const StringList& arguments, ArmipsArguments& settings)
{
	size_t argpos = 0;
	bool readflags = true;
	while (argpos < arguments.size())
	{
		if (readflags && arguments[argpos][0] == L'-')
		{
			if (arguments[argpos] == L"--")
			{
				readflags = false;
				argpos += 1;
			}
			else if (arguments[argpos] == L"-temp" && argpos + 1 < arguments.size())
			{
				settings.tempFileName = arguments[argpos + 1];
				argpos += 2;
			}
			else if (arguments[argpos] == L"-sym" && argpos + 1 < arguments.size())
			{
				settings.symFileName = arguments[argpos + 1];
				settings.symFileVersion = 1;
				argpos += 2;
			}
			else if (arguments[argpos] == L"-sym2" && argpos + 1 < arguments.size())
			{
				settings.symFileName = arguments[argpos + 1];
				settings.symFileVersion = 2;
				argpos += 2;
			}
			else if (arguments[argpos] == L"-erroronwarning")
			{
				settings.errorOnWarning = true;
				argpos += 1;
			}
			else if (arguments[argpos] == L"-equ" && argpos + 2 < arguments.size())
			{
				EquationDefinition def;
				def.name = arguments[argpos + 1];
				std::transform(def.name.begin(), def.name.end(), def.name.begin(), ::towlower);
				def.value = arguments[argpos + 2];
				settings.equList.push_back(def);
				argpos += 3;
			}
			else if (arguments[argpos] == L"-strequ" && argpos + 2 < arguments.size())
			{
				EquationDefinition def;
				def.name = arguments[argpos + 1];
				std::transform(def.name.begin(), def.name.end(), def.name.begin(), ::towlower);
				def.value = formatString(L"\"%s\"", arguments[argpos + 2]);
				settings.equList.push_back(def);
				argpos += 3;
			}
			else if (arguments[argpos] == L"-time")
			{
				Logger::printError(Logger::Warning, L"-time flag is deprecated");
				argpos += 1;
			}
			else if (arguments[argpos] == L"-root" && argpos + 1 < arguments.size())
			{
				changeDirectory(arguments[argpos + 1]);
				argpos += 2;
			}
			else if (arguments[argpos] == L"-definelabel" && argpos + 2 < arguments.size())
			{
				LabelDefinition def;

				def.name = arguments[argpos + 1];
				std::transform(def.name.begin(), def.name.end(), def.name.begin(), ::towlower);

				int64_t value;
				if (!stringToInt(arguments[argpos + 2], 0, arguments[argpos + 2].size(), value))
				{
					Logger::printError(Logger::Error, L"Invalid definelabel value '%s'\n", arguments[argpos + 2]);
					printUsage(arguments[0]);
					return false;
				}
				def.value = value;

				settings.labels.push_back(def);
				argpos += 3;
			}
			else {
				Logger::printError(Logger::Error, L"Invalid command line argument '%s'\n", arguments[argpos]);
				printUsage(arguments[0]);
				return false;
			}
		}
		else {
			// only allow one input filename
			if (settings.inputFileName == L"")
			{
				settings.inputFileName = arguments[argpos];
				argpos++;
			}
			else {
				Logger::printError(Logger::Error, L"Multiple input assembly files specified\n");
				printUsage(arguments[0]);
				return false;
			}
		}
	}

	// ensure input file was specified
	if (settings.inputFileName == L"")
	{
		if (arguments.size() > 1)
			Logger::printError(Logger::Error, L"Missing input assembly file\n");

		printUsage(arguments[0]);
		return false;
	}

	// turn input filename into an absolute path
	if (settings.useAbsoluteFileNames && isAbsolutePath(settings.inputFileName) == false)
		settings.inputFileName = formatString(L"%s/%s", getCurrentDirectory(), settings.inputFileName);

	if (fileExists(settings.inputFileName) == false)
	{
		Logger::printError(Logger::Error, L"File '%s' not found\n", settings.inputFileName);
		return false;
	}
	return true;
}

int runFromCommandLine(const StringList& arguments, ArmipsArguments settings)
{
	if (parseArguments(arguments, settings) == false)
	{
		if (!settings.silent)
			Logger::printLine(L"Cannot parse arguments; aborting.");

		return 1;
	}

	if (runArmips(settings) == false)
	{
		if (!settings.silent)
			Logger::printLine(L"Aborting.");

		return 1;
	}

	return 0;
}
