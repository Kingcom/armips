#include "stdafx.h"
#include "Core/Common.h"
#include "Core/Assembler.h"
#include "Archs/MIPS/Mips.h"
#include "Commands/CDirectiveFile.h"
#include "Tests.h"
#include <chrono>

#if defined(_WIN64) || defined(__x86_64__) || defined(__amd64__)
#define ARMIPSNAME "ARMIPS64"
#else
#define ARMIPSNAME "ARMIPS"
#endif

typedef std::chrono::steady_clock Clock;

int wmain(int argc, wchar_t* argv[])
{
	ArmipsArguments parameters;
   
	auto startTime = Clock::now();

#ifdef ARMIPS_TESTS
	std::wstring name;

	if (argc < 2)
		return !runTests(L"Tests");
	else
		return !runTests(argv[1]);
#endif

	Logger::printLine(L"%s Assembler v%d.%d.%d (" __DATE__ " " __TIME__ ") by Kingcom",
		ARMIPSNAME,ARMIPS_VERSION_MAJOR,ARMIPS_VERSION_MINOR,ARMIPS_VERSION_REVISION);

	StringList arguments = getStringListFromArray(argv,argc);

	if (arguments.size() < 2)
	{
		Logger::printLine(L"Usage: %s file.asm [-temp temp.txt] [-sym symfile.sym]", argv[0]);
		return 1;
	}

	parameters.inputFileName = arguments[1];
	if (fileExists(parameters.inputFileName) == false)
	{
		Logger::printLine(L"File %S not found\n",parameters.inputFileName);
		return 1;
	}

	size_t argpos = 2;
	bool printTime = false;
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
		} else if (arguments[argpos] == L"-erroronwarning")
		{
			parameters.errorOnWarning = true;
			argpos += 1;
		} else if (arguments[argpos] == L"-equ")
		{
			EquationDefinition def;
			def.name = arguments[argpos + 1];
			def.value = arguments[argpos + 2];
			parameters.equList.push_back(def);
			argpos += 3;
		} else if (arguments[argpos] == L"-strequ")
		{
			EquationDefinition def;
			def.name = arguments[argpos + 1];
			def.value = formatString(L"\"%s\"",arguments[argpos + 2]);
			parameters.equList.push_back(def);
			argpos += 3;
		} else if (arguments[argpos] == L"-time")
		{
			printTime = true;
			argpos += 1;
		} else {
			Logger::printLine(L"Invalid parameter %S\n",arguments[argpos]);
			return 1;
		}
	}

	bool result = runArmips(parameters);
	
	auto endTime = Clock::now();
	auto executionTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime-startTime);
	if (result == false)
	{
		if (printTime)
			Logger::printLine(L"Aborting. Duration: %.3fs",executionTime.count()/1000.);
		else
			Logger::printLine(L"Aborting.");
		return 1;
	}
	
	if (printTime)
		Logger::printLine(L"Done. Duration: %.3fs",executionTime.count()/1000.);
	else
		Logger::printLine(L"Done.");
	return 0;
}

#ifndef _WIN32

int main(int argc, char* argv[])
{
	// convert input to wstring
	std::vector<std::wstring> wideStrings;
	for (int i = 0; i < argc; i++)
	{
		std::wstring str = convertUtf8ToWString(argv[i]);
		wideStrings.push_back(str);
	}

	// create argv replacement
	wchar_t** wargv = new wchar_t*[argc];
	for (int i = 0; i < argc; i++)
	{
		wargv[i] = (wchar_t*) wideStrings[i].c_str();
	}

	int result = wmain(argc,wargv);

	delete[] wargv;
	return result;
}

#endif
