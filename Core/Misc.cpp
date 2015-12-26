#include "stdafx.h"
#include "Misc.h"
#include "Common.h"
#include "Core/FileManager.h"
#include <iostream>

#ifdef _MSC_VER
#include <Windows.h>
#endif

std::vector<Logger::QueueEntry> Logger::queue;
std::vector<std::wstring> Logger::errors;
bool Logger::error = false;
bool Logger::fatalError = false;
bool Logger::errorOnWarning = false;
bool Logger::silent = false;

std::wstring Logger::formatError(ErrorType type, const std::wstring& text)
{
	const std::wstring& fileName = Global.memoryMode ? L"<memory>" : Global.FileInfo.FileList[Global.FileInfo.FileNum];

	switch (type)
	{
	case Warning:
		return formatString(L"%s(%d) warning: %s",fileName,Global.FileInfo.LineNumber,text);
	case Error:
		return formatString(L"%s(%d) error: %s",fileName,Global.FileInfo.LineNumber,text);
	case FatalError:
		return formatString(L"%s(%d) fatal error: %s",fileName,Global.FileInfo.LineNumber,text);
	case Notice:
		return formatString(L"%s(%d) notice: %s",fileName,Global.FileInfo.LineNumber,text);
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
	std::wcout << text << std::endl;

#if defined(_MSC_VER) && defined(_DEBUG)
	OutputDebugStringW(text.c_str());
	OutputDebugStringW(L"\n");
#endif
}

void Logger::printLine(const std::string& text)
{
	std::cout << text << std::endl;
	
#if defined(_MSC_VER) && defined(_DEBUG)
	OutputDebugStringA(text.c_str());
	OutputDebugStringA("\n");
#endif
}

void Logger::print(const std::wstring& text)
{
	std::wcout << text;
	
#if defined(_MSC_VER) && defined(_DEBUG)
	OutputDebugStringW(text.c_str());
#endif
}

void Logger::printError(ErrorType type, const std::wstring& text)
{
	std::wstring errorText = formatError(type,text);
	errors.push_back(errorText);

	if (!silent)
		printLine(errorText);

	setFlags(type);
}

void Logger::queueError(ErrorType type, const std::wstring& text)
{
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
	if (file.getFileName().empty() == false)
	{
		if (file.open(TextFile::Write) == false)
		{
			Logger::printError(Logger::Error,L"Could not open temp file %s.",file.getFileName());
			return;
		}

		size_t fileCount = Global.FileInfo.FileList.size();
		size_t lineCount = Global.FileInfo.TotalLineCount;
		size_t labelCount = Global.symbolTable.getLabelCount();
		size_t equCount = Global.symbolTable.getEquationCount();

		file.writeFormat(L"; %d %S included\n",fileCount,fileCount == 1 ? "file" : "files");
		file.writeFormat(L"; %d %S\n",lineCount,lineCount == 1 ? "line" : "lines");
		file.writeFormat(L"; %d %S\n",labelCount,labelCount == 1 ? "label" : "labels");
		file.writeFormat(L"; %d %S\n\n",equCount,equCount == 1 ? "equation" : "equations");
		for (size_t i = 0; i < fileCount; i++)
		{
			file.writeFormat(L"; %S\n",Global.FileInfo.FileList[i]);
		}
		file.writeLine("");
	}
}

void TempData::end()
{
	if (file.isOpen())
		file.close();
}

void TempData::writeLine(u64 memoryAddress, const std::wstring& text)
{
	if (file.isOpen())
	{
		wchar_t hexbuf[10] = {0};
		swprintf(hexbuf, 10, L"%08X ", memoryAddress);
		std::wstring str = hexbuf + text;
		while (str.size() < 70)
			str += ' ';

		str += formatString(L"; %S line %d",
			Global.FileInfo.FileList[Global.FileInfo.FileNum],Global.FileInfo.LineNumber);

		file.writeLine(str);
	}
}
