#pragma once
#include <vector>
#include "Util/Util.h"

class Logger
{
public:
	enum ErrorType { Warning, Error, FatalError, Notice };

	static void clear();
	static void printLine(const std::wstring& text);
	static void printLine(const std::string& text);
	
	template <typename... Args>
	static void printLine(const wchar_t* text, const Args&... args)
	{
		std::wstring message = formatString(text,args...);
		printLine(message);
	}

	static void print(const std::wstring& text);
	
	template <typename... Args>
	static void print(const wchar_t* text, const Args&... args)
	{
		std::wstring message = formatString(text,args...);
		print(message);
	}

	static void printError(ErrorType type, const std::wstring& text);
	static void queueError(ErrorType type, const std::wstring& text);

	template <typename... Args>
	static void printError(ErrorType type, const wchar_t* text, const Args&... args)
	{
		std::wstring message = formatString(text,args...);
		printError(type,message);
	}
	
	template <typename... Args>
	static void queueError(ErrorType type, const wchar_t* text, const Args&... args)
	{
		std::wstring message = formatString(text,args...);
		queueError(type,message);
	}

	static void printQueue();
	static void clearQueue() { queue.clear(); };
	static StringList getErrors() { return errors; };
	static bool hasError() { return error; };
	static bool hasFatalError() { return fatalError; };
	static void setErrorOnWarning(bool b) { errorOnWarning = b; };
	static void setSilent(bool b) { silent = b; };
	static bool isSilent() { return silent; }
private:
	static std::wstring formatError(ErrorType type, const std::wstring& text);
	static void setFlags(ErrorType type);

	struct QueueEntry
	{
		ErrorType type;
		std::wstring text;
	};

	static std::vector<QueueEntry> queue;
	static std::vector<std::wstring> errors;
	static bool error;
	static bool fatalError;
	static bool errorOnWarning;
	static bool silent;
};

class ConditionData
{
public:
	void clear() { conditions.clear(); };
	void addIf(bool conditionMet);
	void addElse();
	void addElseIf(bool conditionMet);
	void addEndIf();
	bool conditionTrue();
	size_t activeConditions() { return conditions.size(); };
private:
	struct Entry
	{
		bool currentConditionMet;
		bool matchingCaseExecuted;
		bool isInElseCase;
	};

	std::vector<Entry> conditions;
};

class AreaData
{
public:
	void clear() { entries.clear(); };
	void startArea(u64 start, size_t size, int fileNum, int lineNumber, int fillValue);
	void endArea();
	bool checkAreas();
	int getCurrentFillValue() { return entries.back().fillValue; };
	u64 getCurrentMaxAddress() { return entries.back().maxAddress; };
	size_t getEntryCount() { return entries.size(); };
private:
	struct Entry
	{
		u64 start;
		u64 maxAddress;
		int fileNum;
		int lineNumber;
		bool overflow;
		int fillValue;
	};

	std::vector<Entry> entries;
};