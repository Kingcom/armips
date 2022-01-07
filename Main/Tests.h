#pragma once

#include "Util/FileSystem.h"

#include <string>
#include <vector>

class TestRunner
{
public:
	bool runTests(const fs::path& dir, const std::string& executableName);
private:
	enum class ConsoleColors { White, Red, Green };

	std::string executableName;
	
	std::vector<fs::path> getTestsList(const fs::path& dir);
	bool executeTest(const fs::path& dir, const std::string& testName, std::string& errorString);
	void changeConsoleColor(ConsoleColors color);
};

bool runTests(const fs::path& dir, const std::string& executableName);
