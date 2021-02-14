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

std::vector<std::wstring> TestRunner::listSubfolders(const std::wstring& dir)
{
	std::vector<std::wstring> result;
	
#ifdef _WIN32
	WIN32_FIND_DATAW findFileData;
	HANDLE hFind;

	std::wstring m = dir + L"*";
	hFind = FindFirstFileW(m.c_str(),&findFileData);
	
	if (hFind != INVALID_HANDLE_VALUE) 
	{
		do
		{
			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				std::wstring dirName = findFileData.cFileName;
				if (dirName != L"." && dirName != L"..")
					result.push_back(dirName);
			}
			
		} while (FindNextFileW(hFind,&findFileData));
	}
#else
	std::string utf8 = convertWStringToUtf8(dir);
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
				std::wstring dirName = convertUtf8ToWString(elem->d_name);
				if (dirName != L"." && dirName != L"..")
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
		Logger::print(L"\033[1;0m");
		break;
	case ConsoleColors::Red:
		Logger::print(L"\033[1;31m");
		break;
	case ConsoleColors::Green:
		Logger::print(L"\033[1;32m");
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

std::vector<std::wstring> TestRunner::getTestsList(const std::wstring& dir, const std::wstring& prefix)
{
	std::vector<std::wstring> tests;

	std::vector<std::wstring> dirs = listSubfolders(dir+prefix);
	for (std::wstring& dirName: dirs)
	{
		std::wstring testName = prefix + dirName;
		std::wstring fileName = dir + testName + L"/" + dirName + L".asm";

		if (fs::exists(fileName))
		{
			if (testName[0] == L'/')
				testName.erase(0,1);
			tests.push_back(testName);
		} else {
			std::vector<std::wstring> subTests = getTestsList(dir,testName+L"/");
			tests.insert(tests.end(),subTests.begin(),subTests.end());
		}
	}

	return tests;
}

bool TestRunner::executeTest(const std::wstring& dir, const std::wstring& testName, std::wstring& errorString)
{
	fs::path oldDir = fs::current_path();
	fs::current_path(dir);

	ArmipsArguments settings;
	std::vector<std::wstring> errors;
	int expectedRetVal = 0;
	int retVal = 0;
	bool checkRetVal = false;
	bool result = true;
	std::vector<std::wstring> args;

	if (fs::exists("commandLine.txt"))
	{
		TextFile f;
		f.open("commandLine.txt", TextFile::Read);
		std::wstring command = f.readLine();
		f.close();
		
		args = splitString(command,L' ',true);
		checkRetVal = true;
		
		// first word is error code, rest is arguments
		expectedRetVal = std::stoi(args[0]);
		args[0] = this->executableName;
	}
	else
	{
		settings.inputFileName = testName + L".asm";
		settings.tempFileName = testName + L".temp.txt";
	}

	settings.errorsResult = &errors;
	settings.silent = true;
	settings.useAbsoluteFileNames = false;

	// may or may not be supposed to cause errors
	retVal = runFromCommandLine(args, settings);

	if (checkRetVal && retVal != expectedRetVal)
	{
		errorString += tfm::format(L"Exit code did not match: expected %S, got %S\n",expectedRetVal,retVal);
		result = false;
	}

	// check errors
	if (fs::exists("expected.txt"))
	{
		TextFile f;
		f.open("expected.txt", TextFile::Read);
		std::vector<std::wstring> expectedErrors = f.readAll();

		if (errors.size() == expectedErrors.size())
		{
			for (size_t i = 0; i < errors.size(); i++)
			{
				if (errors[i] != expectedErrors[i])
				{
					errorString += tfm::format(L"Unexpected error: %S\n",errors[i]);
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
			errorString += tfm::format(L"Unexpected error: %S\n",errors[i]);
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
				errorString += tfm::format(L"Output data does not match\n");
				result = false;
			}
		} else {
			errorString += tfm::format(L"Output data size does not match\n");
			result = false;
		}
	}

	fs::current_path(oldDir);
	return result;
}

bool TestRunner::runTests(const std::wstring& dir, const std::wstring& executableName)
{
	this->executableName = executableName;

	std::vector<std::wstring> tests = getTestsList(dir);
	if (tests.empty())
	{
		Logger::printLine(L"No tests to run");
		return true;
	}

	initConsole();

	unsigned int successCount = 0;
	for (size_t i = 0; i < tests.size(); i++)
	{
		changeConsoleColor(ConsoleColors::White);

		std::wstring line = tfm::format(L"Test %d of %d, %s:",i+1,tests.size(),tests[i]);
		Logger::print(L"%-60s",line);

		std::wstring path = dir + L"/" + tests[i];
		std::wstring errors;

		size_t n = tests[i].find_last_of('/');
		std::wstring testName = n == tests[i].npos ? tests[i] : tests[i].substr(n+1);
		if (!executeTest(path,testName,errors))
		{
			changeConsoleColor(ConsoleColors::Red);
			Logger::printLine(L"FAILED");
			Logger::print(L"%s",errors);
		} else {
			changeConsoleColor(ConsoleColors::Green);
			Logger::printLine(L"PASSED");
			successCount++;
		}
	}
	
	changeConsoleColor(ConsoleColors::White);
	Logger::printLine(L"\n%d out of %d tests passed.",successCount,tests.size());
	
	restoreConsole();
	return successCount == tests.size();
}

bool runTests(const std::wstring& dir, const std::wstring& executableName)
{
	TestRunner runner;
	return runner.runTests(dir, executableName);
}
