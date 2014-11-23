#include "stdafx.h"
#include "Misc.h"
#include "Common.h"
#include "Core/FileManager.h"
#include <iostream>

std::vector<Logger::QueueEntry> Logger::queue;
std::vector<std::wstring> Logger::errors;
bool Logger::error = false;
bool Logger::fatalError = false;
bool Logger::errorOnWarning = false;
bool Logger::silent = false;

std::wstring Logger::formatError(ErrorType type, const std::wstring& text)
{
	const char* fileName = Global.FileInfo.FileList.GetEntry(Global.FileInfo.FileNum);

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
}

void Logger::printLine(const std::string& text)
{
	std::cout << text << std::endl;
}

void Logger::print(const std::wstring& text)
{
	std::wcout << text;
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

void ConditionData::addIf(bool conditionMet)
{
	Entry entry;
	entry.currentConditionMet = conditionMet;
	entry.matchingCaseExecuted = conditionMet;
	entry.isInElseCase = false;
	conditions.push_back(entry);
}

void ConditionData::addElse()
{
	if (conditions.size() == 0)
	{
		Logger::queueError(Logger::Error,L"No if clause active");
		return;
	}

	Entry& entry = conditions.back();
	if (entry.isInElseCase)
	{
		Logger::queueError(Logger::Error,L"Else case already defined");
		return;
	}

	entry.currentConditionMet = !entry.matchingCaseExecuted;
	entry.isInElseCase = true;
}

void ConditionData::addElseIf(bool conditionMet)
{
	if (conditions.size() == 0)
	{
		Logger::queueError(Logger::Error,L"No if clause active");
		return;
	}

	Entry& entry = conditions.back();
	if (entry.isInElseCase)
	{
		Logger::queueError(Logger::Error,L"Else case already defined");
		return;
	}

	if (entry.matchingCaseExecuted)
	{
		entry.currentConditionMet = false;
	} else {
		entry.currentConditionMet = conditionMet;
		entry.matchingCaseExecuted = conditionMet;
	}
}

void ConditionData::addEndIf()
{
	if (conditions.size() == 0)
	{
		Logger::queueError(Logger::Error,L"No if clause active");
		return;
	}

	conditions.pop_back();
}

bool ConditionData::conditionTrue()
{
	for (size_t i = 0; i < conditions.size(); i++)
	{
		if (conditions[i].currentConditionMet == false)
			return false;
	}

	return true;
}


void AreaData::startArea(u64 start, size_t size, int fileNum, int lineNumber, int fillValue)
{
	Entry entry;
	entry.start = start;
	entry.maxAddress = start+size;
	entry.fileNum = fileNum;
	entry.lineNumber = lineNumber;
	entry.overflow = false;
	entry.fillValue = fillValue;
	entries.push_back(entry);
}

void AreaData::endArea()
{
	if (entries.size() == 0)
	{
		Logger::queueError(Logger::Error,L"No active area");
		return;
	}

	entries.pop_back();
}

bool AreaData::checkAreas()
{
	bool error = false;

	for (size_t i = 0; i < entries.size(); i++)
	{
		if ((size_t)entries[i].maxAddress < g_fileManager->getVirtualAddress())
		{
			error = true;
			if (entries[i].overflow == false)
			{
				Logger::queueError(Logger::Error,L"Area at %S(%d) overflown",
					Global.FileInfo.FileList.GetEntry(entries[i].fileNum),
					entries[i].lineNumber);
				entries[i].overflow = true;
			}
		}
	}

	return error;
}
