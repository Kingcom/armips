#include "Core/Common.h"
#include "Archs/MIPS/Mips.h"
#include "Commands/CDirectiveFile.h"
#include "Core/Assembler.h"
#include "Main/CommandLineInterface.h"
#include "Main/Tests.h"
#include "Util/Util.h"

#include <clocale>

int wmain(int argc, wchar_t* argv[])
{
	std::setlocale(LC_CTYPE,"");

#ifdef ARMIPS_TESTS
	std::wstring name;

	if (argc < 2)
		return !runTests(L"Tests", argv[0]);
	else
		return !runTests(argv[1], argv[0]);
#endif

	std::vector<std::wstring> arguments = getStringListFromArray(argv,argc);
	
	return runFromCommandLine(arguments);
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
