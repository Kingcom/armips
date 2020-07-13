#pragma once

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
	bool runTests(const std::wstring& dir, const std::wstring& executableName);
private:
	enum class ConsoleColors { White, Red, Green };

	std::wstring executableName;
	
	std::vector<std::wstring> getTestsList(const std::wstring& dir, const std::wstring& prefix = L"/");
	bool executeTest(const std::wstring& dir, const std::wstring& testName, std::wstring& errorString);
	std::vector<std::wstring> listSubfolders(const std::wstring& dir);
	void initConsole();
	void changeConsoleColor(ConsoleColors color);
	void restoreConsole();

#ifdef _WIN32
	HANDLE hstdin;
	HANDLE hstdout;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
#endif

};

bool runTests(const std::wstring& dir, const std::wstring& executableName);
