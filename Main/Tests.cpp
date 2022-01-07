#include "Main/Tests.h"

#include "Core/Assembler.h"
#include "Core/Common.h"
#include "Core/Misc.h"
#include "Main/CommandLineInterface.h"
#include "Util/FileSystem.h"
#include "Util/Util.h"

#include <cstring>

#ifndef _WIN32
#include <dirent.h>
#endif

std::vector<std::string> TestRunner::listSubfolders(const fs::path& dir)
{
	std::vector<std::string> result;
	
#ifdef _WIN32
	WIN32_FIND_DATAW findFileData;
	HANDLE hFind;

	std::wstring m = dir.wstring() + L"/*";
	hFind = FindFirstFileW(m.c_str(),&findFileData);
	
	if (hFind != INVALID_HANDLE_VALUE) 
	{
		do
		{
			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				std::wstring dirName = findFileData.cFileName;
				if (dirName != L"." && dirName != L"..")
					result.push_back(convertWStringToUtf8(dirName));
			}
			
		} while (FindNextFileW(hFind,&findFileData));
	}
#else
	std::string utf8 = dir.string();
	auto directory = opendir(utf8.c_str());

	if (directory != nullptr)
	{
		auto elem = readdir(directory);
		while (elem != nullptr)
		{
#if defined(__HAIKU__)
			// dirent in Posix does not have a d_type
			struct stat s;
			stat(elem->d_name, &s);
			if (s.st_mode & S_IFDIR)
#else
			if (elem->d_type == DT_DIR)
#endif
			{
				std::string dirName = elem->d_name;
				if (dirName != "." && dirName != "..")
					result.push_back(dirName);
			}

			elem = readdir(directory);
		}
	}
#endif

	return result;
}

void TestRunner::initConsole()
{
#ifdef _WIN32
	// initialize console
	hstdin = GetStdHandle(STD_INPUT_HANDLE);
	hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

	// Remember how things were when we started
	GetConsoleScreenBufferInfo(hstdout,&csbi);
#endif
}

void TestRunner::changeConsoleColor(ConsoleColors color)
{
#ifdef _WIN32
	switch (color)
	{
	case ConsoleColors::White:
		SetConsoleTextAttribute(hstdout,0x7);
		break;
	case ConsoleColors::Red:
		SetConsoleTextAttribute(hstdout,(1 << 2) | (1 << 3));
		break;
	case ConsoleColors::Green:
		SetConsoleTextAttribute(hstdout,(1 << 1) | (1 << 3));
		break;
	}
#else
	switch (color)
	{
	case ConsoleColors::White:
		Logger::print("\033[1;0m");
		break;
	case ConsoleColors::Red:
		Logger::print("\033[1;31m");
		break;
	case ConsoleColors::Green:
		Logger::print("\033[1;32m");
		break;
	}
#endif
}

void TestRunner::restoreConsole()
{
#ifdef _WIN32
	FlushConsoleInputBuffer(hstdin);
	SetConsoleTextAttribute(hstdout,csbi.wAttributes);
#endif
}

std::vector<fs::path> TestRunner::getTestsList(const fs::path& dir)
{
	std::vector<fs::path> tests;

	std::vector<std::string> dirs = listSubfolders(dir);
	for (std::string& dirName: dirs)
	{
		fs::path testDir = dir / dirName;
		fs::path fileName = testDir / (dirName + ".asm");

		if (fs::exists(fileName))
		{
			tests.push_back(testDir);
		} else {
			std::vector<fs::path> subTests = getTestsList(testDir);
			tests.insert(tests.end(),subTests.begin(),subTests.end());
		}
	}

	return tests;
}

std::vector<std::string> splitString(const std::string& str, const char delim, bool skipEmpty)
{
	std::vector<std::string> result;
	std::stringstream stream(str);
	std::string arg;
	while (std::getline(stream,arg,delim))
	{
		if (arg.empty() && skipEmpty) continue;
		result.push_back(arg);
	}

	return result;
}

bool TestRunner::executeTest(const fs::path& dir, const std::string& testName, std::string& errorString)
{
	fs::path oldDir = fs::current_path();
	fs::current_path(dir);

	ArmipsArguments settings;
	std::vector<std::string> errors;
	int expectedRetVal = 0;
	int retVal = 0;
	bool checkRetVal = false;
	bool result = true;
	std::vector<std::string> args;

	if (fs::exists("commandLine.txt"))
	{
		TextFile f;
		f.open("commandLine.txt", TextFile::Read);
		std::string command = f.readLine();
		f.close();
		
		args = splitString(command,L' ',true);
		checkRetVal = true;
		
		// first word is error code, rest is arguments
		expectedRetVal = std::stoi(args[0]);
		args[0] = this->executableName;
	}
	else
	{
		settings.inputFileName = testName + ".asm";
		settings.tempFileName = testName + ".temp.txt";
	}

	settings.errorsResult = &errors;
	settings.silent = true;
	settings.useAbsoluteFileNames = false;

	// may or may not be supposed to cause errors
	retVal = runFromCommandLine(args, settings);

	if (checkRetVal && retVal != expectedRetVal)
	{
		errorString += tfm::format("Exit code did not match: expected %S, got %S\n",expectedRetVal,retVal);
		result = false;
	}

	// check errors
	if (fs::exists("expected.txt"))
	{
		TextFile f;
		f.open("expected.txt", TextFile::Read);
		std::vector<std::string> expectedErrors = f.readAll();

		if (errors.size() == expectedErrors.size())
		{
			for (size_t i = 0; i < errors.size(); i++)
			{
				if (errors[i] != expectedErrors[i])
				{
					errorString += tfm::format("Unexpected error: %S\n",errors[i].data());
					result = false;
				}
			}
		} else {
			result = false;
		}
	} else {
		// if no errors are expected, there should be none
		for (size_t i = 0; i < errors.size(); i++)
		{
			errorString += tfm::format("Unexpected error: %S\n",errors[i].data());
			result = false;
		}
	}

	// write errors to file
	TextFile output;
	output.open("output.txt", TextFile::Write);
	output.writeLines(errors);
	output.close();

	if (fs::exists("expected.bin"))
	{
		ByteArray expected = ByteArray::fromFile("expected.bin");
		ByteArray actual = ByteArray::fromFile("output.bin");

		if (expected.size() == actual.size())
		{
			if (memcmp(expected.data(),actual.data(),actual.size()) != 0)
			{
				errorString += tfm::format("Output data does not match\n");
				result = false;
			}
		} else {
			errorString += tfm::format("Output data size does not match\n");
			result = false;
		}
	}

	fs::current_path(oldDir);
	return result;
}

bool TestRunner::runTests(const fs::path& dir, const std::string& executableName)
{
	this->executableName = executableName;

	std::vector<fs::path> tests = getTestsList(dir);
	if (tests.empty())
	{
		Logger::printLine("No tests to run");
		return true;
	}

	initConsole();

	unsigned int successCount = 0;
	for (size_t i = 0; i < tests.size(); i++)
	{
		changeConsoleColor(ConsoleColors::White);

		std::string line = tfm::format("Test %d of %d, %s:",i+1,tests.size(),tests[i].u8string());
		Logger::print("%-60s",line);

		std::string errors;

		std::string testName = tests[i].filename().u8string();
		if (!executeTest(tests[i],testName,errors))
		{
			changeConsoleColor(ConsoleColors::Red);
			Logger::printLine("FAILED");
			Logger::printLine(errors);
		} else {
			changeConsoleColor(ConsoleColors::Green);
			Logger::printLine("PASSED");
			successCount++;
		}
	}
	
	changeConsoleColor(ConsoleColors::White);
	Logger::printLine("\n%d out of %d tests passed.",successCount,tests.size());
	
	restoreConsole();
	return successCount == tests.size();
}

bool runTests(const fs::path& dir, const std::string& executableName)
{
	TestRunner runner;
	return runner.runTests(dir, executableName);
}
