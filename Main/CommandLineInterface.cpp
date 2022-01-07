#include "Main/CommandLineInterface.h"

#include "Core/Allocations.h"
#include "Core/Assembler.h"
#include "Core/Common.h"
#include "Core/Misc.h"
#include "Util/FileSystem.h"
#include "Util/Util.h"

static void printUsage(const std::string &executableName)
{
	Logger::printLine("armips assembler v%d.%d.%d (%s %s) by Kingcom",
		ARMIPS_VERSION_MAJOR, ARMIPS_VERSION_MINOR, ARMIPS_VERSION_REVISION, __DATE__, __TIME__);
	Logger::printLine("Usage: %s [optional parameters] <FILE>", executableName);
	Logger::printLine("");
	Logger::printLine("Optional parameters:");
	Logger::printLine(" -temp <TEMP>              Output temporary assembly data to <TEMP> file");
	Logger::printLine(" -sym  <SYM>               Output symbol data in the sym format to <SYM> file");
	Logger::printLine(" -sym2 <SYM2>              Output symbol data in the sym2 format to <SYM2> file");
	Logger::printLine(" -root <ROOT>              Use <ROOT> as working directory during execution");
	Logger::printLine(" -equ  <NAME> <VAL>        Equivalent to \'<NAME> equ <VAL>\' in code");
	Logger::printLine(" -strequ <NAME> <VAL>      Equivalent to \'<NAME> equ \"<VAL>\"\' in code");
	Logger::printLine(" -definelabel <NAME> <VAL> Equivalent to \'.definelabel <NAME>, <VAL>\' in code");
	Logger::printLine(" -erroronwarning           Treat all warnings like errors");
	Logger::printLine(" -stat                     Show area usage statistics");
	Logger::printLine("");
	Logger::printLine("File arguments:");
	Logger::printLine(" <FILE>                    Main assembly code file");
}

static bool parseArguments(const std::vector<std::string>& arguments, ArmipsArguments& settings)
{
	size_t argpos = 1;
	bool readflags = true;
	while (argpos < arguments.size())
	{
		if (readflags && arguments[argpos][0] == '-')
		{
			if (arguments[argpos] == "--")
			{
				readflags = false;
				argpos += 1;
			}
			else if (arguments[argpos] == "-temp" && argpos + 1 < arguments.size())
			{
				settings.tempFileName = arguments[argpos + 1];
				argpos += 2;
			}
			else if (arguments[argpos] == "-sym" && argpos + 1 < arguments.size())
			{
				settings.symFileName = arguments[argpos + 1];
				settings.symFileVersion = 1;
				argpos += 2;
			}
			else if (arguments[argpos] == "-sym2" && argpos + 1 < arguments.size())
			{
				settings.symFileName = arguments[argpos + 1];
				settings.symFileVersion = 2;
				argpos += 2;
			}
			else if (arguments[argpos] == "-erroronwarning")
			{
				settings.errorOnWarning = true;
				argpos += 1;
			}
			else if (arguments[argpos] == "-stat")
			{
				settings.showStats = true;
				argpos += 1;
			}
			else if (arguments[argpos] == "-equ" && argpos + 2 < arguments.size())
			{
				Identifier name = Identifier(arguments[argpos+1]);

				if (!checkValidLabelName(name))
				{
					Logger::printError(Logger::Error, "Invalid equation name \"%s\"", name);
					return false;
				}

				auto it = std::find_if(settings.equList.begin(), settings.equList.end(),
						[&name](const EquationDefinition& x) -> bool {return name == x.name;});
				if(it != settings.equList.end())
				{
					Logger::printError(Logger::Error, "Equation name \"%s\" already defined", name);
					return false;
				}

				const std::string &value = arguments[argpos + 2];
				settings.equList.emplace_back(EquationDefinition{name, value});
				argpos += 3;
			}
			else if (arguments[argpos] == "-strequ" && argpos + 2 < arguments.size())
			{
				Identifier name = Identifier(arguments[argpos+1]);

				if (!checkValidLabelName(name))
				{
					Logger::printError(Logger::Error, "Invalid equation name \"%s\"", name);
					return false;
				}

				auto it = std::find_if(settings.equList.begin(), settings.equList.end(),
						[&name](const EquationDefinition& x) -> bool {return name == x.name;});
				if(it != settings.equList.end())
				{
					Logger::printError(Logger::Error, "Equation name \"%s\" already defined", name);
					return false;
				}

				std::string value = tfm::format("\"%s\"", arguments[argpos + 2]);
				settings.equList.emplace_back(EquationDefinition{name, value});
				argpos += 3;
			}
			else if (arguments[argpos] == "-time")
			{
				Logger::printError(Logger::Warning, "-time flag is deprecated");
				argpos += 1;
			}
			else if (arguments[argpos] == "-root" && argpos + 1 < arguments.size())
			{
				std::error_code errorCode;
				fs::current_path(arguments[argpos + 1], errorCode);

				if (errorCode)
				{
					Logger::printError(Logger::Error, "Could not open directory \"%s\"", arguments[argpos + 1]);
					return false;
				}
				argpos += 2;
			}
			else if (arguments[argpos] == "-definelabel" && argpos + 2 < arguments.size())
			{
				Identifier name = Identifier(arguments[argpos + 1]);

				if (!checkValidLabelName(name))
				{
					Logger::printError(Logger::Error, "Invalid label name \"%s\"", name);
					return false;
				}

				auto it = std::find_if(settings.labels.begin(), settings.labels.end(),
						[&name](const LabelDefinition& x) -> bool {return name == x.name;});
				if(it != settings.labels.end())
				{
					Logger::printError(Logger::Error, "Label name \"%s\" already defined", name);
					return false;
				}

				int64_t value;
				if (!stringToInt(arguments[argpos + 2], 0, arguments[argpos + 2].size(), value))
				{
					Logger::printError(Logger::Error, "Invalid label value \"%s\"", arguments[argpos + 2]);
					return false;
				}

				settings.labels.emplace_back(LabelDefinition{name, value});
				argpos += 3;
			}
			else {
				Logger::printError(Logger::Error, "Invalid command line argument \"%s\"\n", arguments[argpos]);
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
				Logger::printError(Logger::Error, "Multiple input assembly files specified\n");
				printUsage(arguments[0]);
				return false;
			}
		}
	}

	// ensure input file was specified
	if (settings.inputFileName.empty())
	{
		if (arguments.size() > 1)
			Logger::printError(Logger::Error, "Missing input assembly file\n");

		printUsage(arguments[0]);
		return false;
	}

	// turn input filename into an absolute path
	if (settings.useAbsoluteFileNames)
		settings.inputFileName = fs::absolute(settings.inputFileName).lexically_normal();

	if (!fs::exists(settings.inputFileName))
	{
		Logger::printError(Logger::Error, "File \"%s\" not found", settings.inputFileName.u8string());
		return false;
	}
	return true;
}

int runFromCommandLine(const std::vector<std::string>& arguments, ArmipsArguments settings)
{
	if (!parseArguments(arguments, settings))
	{
		if (arguments.size() > 1 && !settings.silent)
			Logger::printLine("Cannot parse arguments; aborting.");

		return 1;
	}

	if (!runArmips(settings))
	{
		if (!settings.silent)
			Logger::printLine("Aborting.");

		return 1;
	}

	return 0;
}
