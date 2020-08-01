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

void Logger::formatPreamble(fmt::wmemory_buffer &buffer, ErrorType type)
{
	if (!Global.memoryMode && Global.fileList.size() > 0)
	{
		const auto& fileName = Global.fileList.relativeWstring(Global.FileInfo.FileNum);
		fmt::format_to(buffer, L"{}({}) ", fileName, Global.FileInfo.LineNumber);
	}

	switch (type)
	{
	case Warning:
		fmt::format_to(buffer,L"warning: ");
		break;
	case Error:
		fmt::format_to(buffer,L"error: ");
		break;
	case FatalError:
		fmt::format_to(buffer,L"fatal error: ");
		break;
	case Notice:
		fmt::format_to(buffer,L"notice: ");
		break;
	}
}

void Logger::doPrintError(ErrorType type,std::wstring text)
{
	if (suppressLevel)
		return;

	if (!silent)
		printLine(text);

	errors.emplace_back(std::move(text));

	setFlags(type);
}

void Logger::doQueueError(ErrorType type, std::wstring text)
{
	if (suppressLevel)
		return;

	// save message
	queue.emplace_back(QueueEntry{type, std::move(text)});
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
			Logger::printError(Logger::Error,L"Could not open temp file {}.",file.getFileName().wstring());
			return;
		}

		size_t fileCount = Global.fileList.size();
		size_t lineCount = Global.FileInfo.TotalLineCount;
		size_t labelCount = Global.symbolTable.getLabelCount();
		size_t equCount = Global.symbolTable.getEquationCount();

		file.writeFormat(L"; {} {} included\n",fileCount,fileCount == 1 ? L"file" : L"files");
		file.writeFormat(L"; {} {}\n",lineCount,lineCount == 1 ? L"line" : L"lines");
		file.writeFormat(L"; {} {}\n",labelCount,labelCount == 1 ? L"label" : L"labels");
		file.writeFormat(L"; {} {}\n\n",equCount,equCount == 1 ? L"equation" : L"equations");
		for (size_t i = 0; i < fileCount; i++)
		{
			file.writeFormat(L"; {}\n",Global.fileList.wstring(i));
		}
		file.writeLine("");
	}
}

void TempData::end()
{
	if (file.isOpen())
		file.close();
}

void TempData::writeLine(int64_t memoryAddress, const std::wstring_view& text)
{
	if (file.isOpen())
	{
		fmt::wmemory_buffer data;

		fmt::format_to(data, L"{:08X} {}", uint32_t(memoryAddress), text);

		while (data.size() < 70)
			data.push_back(L' ');

		fmt::format_to(data, L"; {} line {}",
			Global.fileList.wstring(Global.FileInfo.FileNum),Global.FileInfo.LineNumber);

		file.writeLine(data.data());
	}
}
