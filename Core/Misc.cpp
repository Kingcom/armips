#include "stdafx.h"
#include "Misc.h"
#include "Common.h"
#include "Core/FileManager.h"

std::vector<Logger::QueueEntry> Logger::queue;
std::vector<std::wstring> Logger::errors;
bool Logger::error = false;
bool Logger::fatalError = false;
bool Logger::errorOnWarning = false;

std::wstring Logger::formatError(ErrorType type, const std::wstring& text)
{
	std::wstring fileName = convertUtf8ToWString(Global.FileInfo.FileList.GetEntry(Global.FileInfo.FileNum));

	switch (type)
	{
	case Warning:
		return formatString(L"%s(%d) warning: %s",fileName.c_str(),Global.FileInfo.LineNumber,text.c_str());
	case Error:
		return formatString(L"%s(%d) error: %s",fileName.c_str(),Global.FileInfo.LineNumber,text.c_str());
	case FatalError:
		return formatString(L"%s(%d) fatal error: %s",fileName.c_str(),Global.FileInfo.LineNumber,text.c_str());
	case Notice:
		return formatString(L"%s(%d) notice: %s",fileName.c_str(),Global.FileInfo.LineNumber,text.c_str());
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
		exit(2);	// TODO: quit in a less drastic way
		break;
	}
}

void Logger::clear()
{
	queue.clear();
	errors.clear();
	error = false;
	fatalError = false;
}

void Logger::printLine(const std::wstring& text)
{
	wprintf(L"%s\n",text.c_str());
}

void Logger::printLine(const char* format, ...)
{
	va_list args;

	va_start(args,format);
	vprintf(format,args);
	va_end(args);
	printf("\n");
}

void Logger::printError(ErrorType type, const std::wstring& text)
{
	std::wstring errorText = formatError(type,text);
	errors.push_back(errorText);
	printLine(errorText);
	setFlags(type);
}

void Logger::printError(ErrorType type, const wchar_t* format, ...)
{
	std::wstring result;
	va_list args;

	va_start(args,format);

	int length = _vscwprintf(format,args);
	if (length < 0) // error
	{
		va_end(args);
		return;
	}

	wchar_t* buffer = (wchar_t*) alloca((length+1)*sizeof(wchar_t));
	length = _vsnwprintf(buffer,length+1,format,args);

	if (length < 0)
		return;
	va_end(args);

	result = buffer;
	printError(type,result);
}

void Logger::queueError(ErrorType type, const std::wstring& text)
{
	QueueEntry entry;
	entry.type = type;
	entry.text = formatError(type,text);
	queue.push_back(entry);
}

void Logger::queueError(ErrorType type, const wchar_t* format, ...)
{
	std::wstring result;
	va_list args;

	va_start(args,format);

	int length = _vscwprintf(format,args);
	if (length < 0) // error
	{
		va_end(args);
		return;
	}

	wchar_t* buffer = (wchar_t*) alloca((length+1)*sizeof(wchar_t));
	length = _vsnwprintf(buffer,length+1,format,args);

	if (length < 0)
		return;
	va_end(args);

	result = buffer;
	queueError(type,result);
}

void Logger::printQueue()
{
	for (size_t i = 0; i < queue.size(); i++)
	{
		errors.push_back(queue[i].text);
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


void AreaData::startArea(int start, int size, int fileNum, int lineNumber, int fillValue)
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
