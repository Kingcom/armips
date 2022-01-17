#include "Core/Misc.h"

#include "Core/Common.h"
#include "Core/FileManager.h"
#include "Util/FileSystem.h"
#include "Util/Util.h"

#include <iostream>

std::vector<Logger::QueueEntry> Logger::queue;
std::vector<std::string> Logger::errors;
bool Logger::error = false;
bool Logger::fatalError = false;
bool Logger::errorOnWarning = false;
bool Logger::silent = false;
int Logger::suppressLevel = 0;

std::string Logger::formatError(ErrorType type, const char* text)
{
	std::string position;

	if (!Global.memoryMode && Global.fileList.size() > 0)
	{
		const auto& fileName = Global.fileList.relativeString(Global.FileInfo.FileNum);
		position = tfm::format("%s(%d) ", fileName, Global.FileInfo.LineNumber);
	}

	switch (type)
	{
	case Warning:
		return tfm::format("%swarning: %s",position,text);
	case Error:
		return tfm::format("%serror: %s",position,text);
	case FatalError:
		return tfm::format("%sfatal error: %s",position,text);
	case Notice:
		return tfm::format("%snotice: %s",position,text);
	}

	return "";
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

void Logger::printLine(std::string_view text)
{
	if (suppressLevel)
		return;

	std::cout << text << std::endl;
}

void Logger::print(std::string_view text)
{
	if (suppressLevel)
		return;

	std::cout << text;
}

void Logger::printError(ErrorType type, const std::string_view text)
{
	if (suppressLevel)
		return;

	std::string errorText = formatError(type,text.data());
	errors.push_back(errorText);

	if (!silent)
		printLine(errorText);

	setFlags(type);
}

void Logger::queueError(ErrorType type, const std::string_view text)
{
	if (suppressLevel)
		return;

	QueueEntry entry;
	entry.type = type;
	entry.text = formatError(type,text.data());
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
			Logger::printError(Logger::Error,"Could not open temp file %s.",file.getFileName().u8string());
			return;
		}

		size_t fileCount = Global.fileList.size();
		size_t lineCount = Global.FileInfo.TotalLineCount;
		size_t labelCount = Global.symbolTable.getLabelCount();
		size_t equCount = Global.symbolTable.getEquationCount();

		file.writeFormat("; %d %S included\n",fileCount,fileCount == 1 ? "file" : "files");
		file.writeFormat("; %d %S\n",lineCount,lineCount == 1 ? "line" : "lines");
		file.writeFormat("; %d %S\n",labelCount,labelCount == 1 ? "label" : "labels");
		file.writeFormat("; %d %S\n\n",equCount,equCount == 1 ? "equation" : "equations");
		for (size_t i = 0; i < fileCount; i++)
		{
			file.writeFormat("; %S\n",Global.fileList.string(int(i)));
		}
		file.writeLine("");
	}
}

void TempData::end()
{
	if (file.isOpen())
		file.close();
}

void TempData::writeLine(int64_t memoryAddress, const std::string& text)
{
	if (file.isOpen())
	{
		char hexbuf[10] = {0};
		snprintf(hexbuf, 10, "%08X ", (int32_t) memoryAddress);
		std::string str = hexbuf + text;
		while (str.size() < 70)
			str += ' ';

		str += tfm::format("; %S line %d",
			Global.fileList.string(Global.FileInfo.FileNum),Global.FileInfo.LineNumber);

		file.writeLine(str);
	}
}
