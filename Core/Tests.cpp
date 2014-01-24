#include "stdafx.h"
#include "Tests.h"
#include "Util/Util.h"
#include "Util/CommonClasses.h"
#include "Core/Common.h"
#include "Assembler.h"
#include <direct.h>

StringList getTestsList(const std::wstring& dir)
{
	StringList tests;

	WIN32_FIND_DATA findFileData;
	HANDLE hFind;

	std::wstring m = dir + L"/*";
	hFind = FindFirstFile(m.c_str(),&findFileData);
	if (hFind != INVALID_HANDLE_VALUE) 
	{
		do
		{
			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				std::wstring dirName = findFileData.cFileName;
				if (dirName != L"." && dirName != L"..")
					tests.push_back(dirName);
			}
		} while (FindNextFile(hFind,&findFileData));
	}

	return tests;
}

bool executeTest(const std::wstring& dir, const std::wstring& testName, std::string& errorString)
{
	wchar_t oldDir[MAX_PATH];
	_wgetcwd(oldDir,MAX_PATH-1);
	_wchdir(dir.c_str());

	AssemblerArguments args;
	StringList errors;

	args.inputFileName = testName + L".asm";
	args.errorsResult = &errors;
	args.silent = true;

	// may or may not be supposed to cause errors
	runAssembler(args);

	// check errors
	bool result = true;
	if (fileExists(L"expected.txt"))
	{
		TextFile f;
		f.open(L"expected.txt",TextFile::Read);
		StringList expectedErrors = f.readAll();

		if (errors.size() == expectedErrors.size())
		{
			for (size_t i = 0; i < errors.size(); i++)
			{
				if (errors[i] != expectedErrors[i])
				{
					errorString += formatString("Unexpected error: %S\n",errors[i].c_str());
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
			errorString += formatString("Unexpected error: %S\n",errors[i].c_str());
			result = false;
		}
	}

	if (fileExists(L"expected.bin"))
	{
		ByteArray expected = ByteArray::fromFile(L"expected.bin");
		ByteArray actual = ByteArray::fromFile(L"output.bin");

		if (expected.size() == actual.size())
		{
			if (memcmp(expected.data(),actual.data(),actual.size()) != 0)
			{
				errorString += formatString("Output data does not match\n");
				result = false;
			}
		} else {
			errorString += formatString("Output data size does not match\n");
			result = false;
		}

		DeleteFile(L"output.bin");
	}

	_wchdir(oldDir);
	return result;
}

bool runTests(const std::wstring& dir)
{
	StringList tests = getTestsList(dir);
	if (tests.empty())
	{
		Logger::printLine("No tests to run");
		return true;
	}

	int successCount = 0;
	for (size_t i = 0; i < tests.size(); i++)
	{
		std::string line = formatString("Test %d of %d, %S:",i+1,tests.size(),tests[i].c_str());
		printf("%-50s",line.c_str());

		std::wstring path = dir + L"/" + tests[i];
		std::string errors;

		if (executeTest(path,tests[i],errors) == false)
		{
			printf("FAILED\n");
			printf("%s",errors.c_str());
		} else {
			printf("PASSED\n");
			successCount++;
		}
	}

	Logger::printLine("\n%d out of %d tests passed.\n",successCount,tests.size());
	
	return successCount == tests.size();
}