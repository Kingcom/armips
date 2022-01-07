#include "Main/CommandLineInterface.h"
#include "Main/Tests.h"
#include "Util/Util.h"

#include <clocale>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace
{
	int run(const std::vector<std::string> &arguments)
	{
#ifdef ARMIPS_TESTS
		if (arguments.size() < 2)
			return !runTests("Tests", arguments[0]);
		else
			return !runTests(arguments[1], arguments[0]);
#endif
		return runFromCommandLine(arguments);
	}
}

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[])
{
	std::setlocale(LC_CTYPE, "");

	// enable ANSI escape code processing
	HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD consoleMode = 0;
	GetConsoleMode(stdoutHandle , &consoleMode);
	consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(stdoutHandle, consoleMode);

	// process parameters
	std::vector<std::string> arguments;
	for (int i = 0; i < argc; ++i)
	{
		arguments.push_back(convertWStringToUtf8(argv[i]));
	}

	return run(arguments);
}
#else
int main(int argc, char* argv[])
{
	std::setlocale(LC_CTYPE, "");

	std::vector<std::string> arguments;
	for (int i = 0; i < argc; i++)
	{
		arguments.push_back(argv[i]);
	}

	return run(arguments);
}
#endif
