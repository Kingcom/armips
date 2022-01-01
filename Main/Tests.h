#pragma once

#include "Util/FileSystem.h"

#include <string>
#include <vector>

#if defined(__HAIKU__)
#include <sys/stat.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

class TestRunner
{
public:
	bool runTests(const fs::path& dir, const std::string& executableName);
private:
	enum class ConsoleColors { White, Red, Green };

	std::string executableName;
	
	std::vector<fs::path> getTestsList(const fs::path& dir);
	bool executeTest(const fs::path& dir, const std::string& testName, std::string& errorString);
	std::vector<std::string> listSubfolders(const fs::path& dir);
	void initConsole();
	void changeConsoleColor(ConsoleColors color);
	void restoreConsole();

#ifdef _WIN32
	HANDLE hstdin;
	HANDLE hstdout;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
#endif

};

bool runTests(const fs::path& dir, const std::string& executableName);
