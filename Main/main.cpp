#include "stdafx.h"
#include "Core/Common.h"
#include "Core/MathParser.h"
#include "Util/CommonClasses.h"
#include "Core/Assembler.h"
#include "Archs/MIPS/Mips.h"
#include "Commands/CDirectiveFile.h"
#include "Tests.h"

int wmain(int argc, wchar_t* argv[])
{
	ArmipsArguments parameters;

#ifdef ARMIPS_TESTS
	std::wstring name;

	if (argc < 2)
		return !runTests(L"Tests");
	else
		return !runTests(argv[1]);
#endif

	Logger::printLine("ARMIPS Assembler v0.7d (" __DATE__ " " __TIME__ ") by Kingcom");
	StringList arguments = getStringListFromArray(argv,argc);

	if (arguments.size() < 2)
	{
		Logger::printLine(L"Usage: armips.exe file.asm [-temp temp.txt] [-sym symfile.sym]");
		return 1;
	}

	parameters.inputFileName = arguments[1];
	if (fileExists(parameters.inputFileName) == false)
	{
		Logger::printLine(L"File %S not found\n",parameters.inputFileName);
		return 1;
	}

	size_t argpos = 2;
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
			std::wstring name = arguments[argpos + 1];
			std::wstring replacement = arguments[argpos + 2];
			parameters.equList.push_back(name + L" equ " + replacement);
			argpos += 3;
		} else {
			Logger::printLine(L"Invalid parameter %S\n",arguments[argpos]);
			return 1;
		}
	}

	bool result = runArmips(parameters);
	if (result == false)
	{
		Logger::printLine(L"Aborting.");
		return 1;
	}
	
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