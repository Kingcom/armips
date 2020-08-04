#include "Main/CommandLineInterface.h"

#include "Core/Allocations.h"
#include "Core/Assembler.h"
#include "Core/Common.h"
#include "Core/Misc.h"
#include "Util/FileSystem.h"
#include "Util/Util.h"

static void printUsage(std::wstring executableName)
{
	Logger::printLine(L"armips assembler v%d.%d.%d (%s %s) by Kingcom",
		ARMIPS_VERSION_MAJOR, ARMIPS_VERSION_MINOR, ARMIPS_VERSION_REVISION, __DATE__, __TIME__);
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
	Logger::printLine(L" -stat                     Show area usage statistics");
	Logger::printLine(L"");
	Logger::printLine(L"File arguments:");
	Logger::printLine(L" <FILE>                    Main assembly code file");
}

static bool parseArguments(const std::vector<std::wstring>& arguments, ArmipsArguments& settings)
{
	size_t argpos = 1;
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
			else if (arguments[argpos] == L"-stat")
			{
				settings.showStats = true;
				argpos += 1;
			}
			else if (arguments[argpos] == L"-equ" && argpos + 2 < arguments.size())
			{
				EquationDefinition def;

				def.name = arguments[argpos+1];
				std::transform(def.name.begin(), def.name.end(), def.name.begin(), ::towlower);

				if (!checkValidLabelName(def.name))
				{
					Logger::printError(Logger::Error, L"Invalid equation name \"%s\"", def.name);
					return false;
				}

				auto it = std::find_if(settings.equList.begin(), settings.equList.end(),
						[&def](EquationDefinition x) -> bool {return def.name == x.name;});
				if(it != settings.equList.end())
				{
					Logger::printError(Logger::Error, L"Equation name \"%s\" already defined", def.name);
					return false;
				}

				def.value = arguments[argpos + 2];
				settings.equList.push_back(def);
				argpos += 3;
			}
			else if (arguments[argpos] == L"-strequ" && argpos + 2 < arguments.size())
			{
				EquationDefinition def;

				def.name = arguments[argpos+1];
				std::transform(def.name.begin(), def.name.end(), def.name.begin(), ::towlower);

				if (!checkValidLabelName(def.name))
				{
					Logger::printError(Logger::Error, L"Invalid equation name \"%s\"", def.name);
					return false;
				}

				auto it = std::find_if(settings.equList.begin(), settings.equList.end(),
						[&def](EquationDefinition x) -> bool {return def.name == x.name;});
				if(it != settings.equList.end())
				{
					Logger::printError(Logger::Error, L"Equation name \"%s\" already defined", def.name);
					return false;
				}

				def.value = tfm::format(L"\"%s\"", arguments[argpos + 2]);
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
				std::error_code errorCode;
				fs::current_path(arguments[argpos + 1], errorCode);

				if (errorCode)
				{
					Logger::printError(Logger::Error, L"Could not open directory \"%s\"", arguments[argpos + 1]);
					return false;
				}
				argpos += 2;
			}
			else if (arguments[argpos] == L"-definelabel" && argpos + 2 < arguments.size())
			{
				LabelDefinition def;

				def.originalName = arguments[argpos + 1];
				def.name = def.originalName;
				std::transform(def.name.begin(), def.name.end(), def.name.begin(), ::towlower);

				if (!checkValidLabelName(def.name))
				{
					Logger::printError(Logger::Error, L"Invalid label name \"%s\"", def.name);
					return false;
				}

				auto it = std::find_if(settings.labels.begin(), settings.labels.end(),
						[&def](LabelDefinition x) -> bool {return def.name == x.name;});
				if(it != settings.labels.end())
				{
					Logger::printError(Logger::Error, L"Label name \"%s\" already defined", def.name);
					return false;
				}

				int64_t value;
				if (!stringToInt(arguments[argpos + 2], 0, arguments[argpos + 2].size(), value))
				{
					Logger::printError(Logger::Error, L"Invalid label value \"%s\"", arguments[argpos + 2]);
					return false;
				}
				def.value = value;

				settings.labels.push_back(def);
				argpos += 3;
			}
			else {
				Logger::printError(Logger::Error, L"Invalid command line argument \"%s\"\n", arguments[argpos]);
				printUsage(arguments[0]);
				return false;
			}
		}
		else {
			// only allow one input filename
			if (settings.inputFileName.empty())
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
	if (settings.inputFileName.empty())
	{
		if (arguments.size() > 1)
			Logger::printError(Logger::Error, L"Missing input assembly file\n");

		printUsage(arguments[0]);
		return false;
	}

	// turn input filename into an absolute path
	if (settings.useAbsoluteFileNames)
		settings.inputFileName = fs::absolute(settings.inputFileName).lexically_normal();

	if (!fs::exists(settings.inputFileName))
	{
		Logger::printError(Logger::Error, L"File \"%s\" not found", settings.inputFileName);
		return false;
	}
	return true;
}

int runFromCommandLine(const std::vector<std::wstring>& arguments, ArmipsArguments settings)
{
	if (!parseArguments(arguments, settings))
	{
		if (arguments.size() > 1 && !settings.silent)
			Logger::printLine(L"Cannot parse arguments; aborting.");

		return 1;
	}

	if (!runArmips(settings))
	{
		if (!settings.silent)
			Logger::printLine(L"Aborting.");

		return 1;
	}

	return 0;
}
