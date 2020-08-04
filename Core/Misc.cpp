#include "Core/Misc.h"

#include "Core/Common.h"
#include "Core/FileManager.h"
#include "Util/FileSystem.h"

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

std::vector<Logger::QueueEntry> Logger::queue;
std::vector<std::wstring> Logger::errors;
bool Logger::error = false;
bool Logger::fatalError = false;
bool Logger::errorOnWarning = false;
bool Logger::silent = false;
int Logger::suppressLevel = 0;

std::wstring Logger::formatError(ErrorType type, const wchar_t* text)
{
	std::wstring position;

	if (!Global.memoryMode && Global.fileList.size() > 0)
	{
		const auto& fileName = Global.fileList.relativeWstring(Global.FileInfo.FileNum);
		position = tfm::format(L"%s(%d) ", fileName, Global.FileInfo.LineNumber);
	}

	switch (type)
	{
	case Warning:
		return tfm::format(L"%swarning: %s",position,text);
	case Error:
		return tfm::format(L"%serror: %s",position,text);
	case FatalError:
		return tfm::format(L"%sfatal error: %s",position,text);
	case Notice:
		return tfm::format(L"%snotice: %s",position,text);
	}

	return L"";
}

void Logger::setFlags(ErrorType type)
{
	switch (type)
	{
	case Warning:
		if (errorOnWarning)
			error = true;
		break;
	case Error:
		error = true;
		break;
	case FatalError:
		error = true;
		fatalError = true;
		break;
	case Notice:
		break;
	}
}

void Logger::clear()
{
	queue.clear();
	errors.clear();
	error = false;
	fatalError = false;
	errorOnWarning = false;
	silent = false;
}

void Logger::printLine(const std::wstring& text)
{
	if (suppressLevel)
		return;

	std::wcout << text << std::endl;

#if defined(_MSC_VER) && defined(_DEBUG)
	OutputDebugStringW(text.c_str());
	OutputDebugStringW(L"\n");
#endif
}

void Logger::printLine(const std::string& text)
{
	if (suppressLevel)
		return;

	std::cout << text << std::endl;
	
#if defined(_MSC_VER) && defined(_DEBUG)
	OutputDebugStringA(text.c_str());
	OutputDebugStringA("\n");
#endif
}

void Logger::print(const std::wstring& text)
{
	if (suppressLevel)
		return;

	std::wcout << text;
	
#if defined(_MSC_VER) && defined(_DEBUG)
	OutputDebugStringW(text.c_str());
#endif
}

void Logger::printError(ErrorType type, const std::wstring& text)
{
	if (suppressLevel)
		return;

	std::wstring errorText = formatError(type,text.c_str());
	errors.push_back(errorText);

	if (!silent)
		printLine(errorText);

	setFlags(type);
}

void Logger::printError(ErrorType type, const wchar_t* text)
{
	if (suppressLevel)
		return;

	std::wstring errorText = formatError(type,text);
	errors.push_back(errorText);

	if (!silent)
		printLine(errorText);

	setFlags(type);
}

void Logger::queueError(ErrorType type, const std::wstring& text)
{
	if (suppressLevel)
		return;

	QueueEntry entry;
	entry.type = type;
	entry.text = formatError(type,text.c_str());
	queue.push_back(entry);
}

void Logger::queueError(ErrorType type, const wchar_t* text)
{
	if (suppressLevel)
		return;

	QueueEntry entry;
	entry.type = type;
	entry.text = formatError(type,text);
	queue.push_back(entry);
}

void Logger::printQueue()
{
	for (size_t i = 0; i < queue.size(); i++)
	{
		errors.push_back(queue[i].text);

		if (!silent)
			printLine(queue[i].text);

		setFlags(queue[i].type);
	}
}

void TempData::start()
{
	if (!file.getFileName().empty())
	{
		if (!file.open(TextFile::Write))
		{
			Logger::printError(Logger::Error,L"Could not open temp file %s.",file.getFileName());
			return;
		}

		size_t fileCount = Global.fileList.size();
		size_t lineCount = Global.FileInfo.TotalLineCount;
		size_t labelCount = Global.symbolTable.getLabelCount();
		size_t equCount = Global.symbolTable.getEquationCount();

		file.writeFormat(L"; %d %S included\n",fileCount,fileCount == 1 ? "file" : "files");
		file.writeFormat(L"; %d %S\n",lineCount,lineCount == 1 ? "line" : "lines");
		file.writeFormat(L"; %d %S\n",labelCount,labelCount == 1 ? "label" : "labels");
		file.writeFormat(L"; %d %S\n\n",equCount,equCount == 1 ? "equation" : "equations");
		for (size_t i = 0; i < fileCount; i++)
		{
			file.writeFormat(L"; %S\n",Global.fileList.wstring(i));
		}
		file.writeLine("");
	}
}

void TempData::end()
{
	if (file.isOpen())
		file.close();
}

void TempData::writeLine(int64_t memoryAddress, const std::wstring& text)
{
	if (file.isOpen())
	{
		wchar_t hexbuf[10] = {0};
		swprintf(hexbuf, 10, L"%08X ", (int32_t) memoryAddress);
		std::wstring str = hexbuf + text;
		while (str.size() < 70)
			str += ' ';

		str += tfm::format(L"; %S line %d",
			Global.fileList.wstring(Global.FileInfo.FileNum),Global.FileInfo.LineNumber);

		file.writeLine(str);
	}
}
