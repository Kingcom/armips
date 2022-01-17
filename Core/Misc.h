#pragma once

#include "Util/FileClasses.h"
#include "Util/Util.h"

#include <string_view>
#include <vector>

#include <tinyformat.h>

class Logger
{
public:
	enum ErrorType { Warning, Error, FatalError, Notice };

	static void clear();
	static void printLine(std::string_view text);
	
	template <typename... Args>
	static void printLine(const char* text, const Args&... args)
	{
		std::string message = tfm::format(text,args...);
		printLine(message);
	}

	static void print(std::string_view text);
	
	template <typename... Args>
	static void print(const char* text, const Args&... args)
	{
		std::string message = tfm::format(text,args...);
		print(message);
	}

	static void printError(ErrorType type, std::string_view text);
	static void queueError(ErrorType type, std::string_view text);

	template <typename... Args>
	static void printError(ErrorType type, const char* text, const Args&... args)
	{
		std::string message = tfm::format(text,args...);
		printError(type,message);
	}
	
	template <typename... Args>
	static void queueError(ErrorType type, const char* text, const Args&... args)
	{
		std::string message = tfm::format(text,args...);
		queueError(type,message);
	}

	static void printQueue();
	static void clearQueue() { queue.clear(); };
	static std::vector<std::string> getErrors() { return errors; };
	static bool hasError() { return error; };
	static bool hasFatalError() { return fatalError; };
	static void setErrorOnWarning(bool b) { errorOnWarning = b; };
	static void setSilent(bool b) { silent = b; };
	static bool isSilent() { return silent; }
	static void suppressErrors() { ++suppressLevel; }
	static void unsuppressErrors() { if (suppressLevel) --suppressLevel; }
private:
	static std::string formatError(ErrorType type, const char* text);
	static void setFlags(ErrorType type);

	struct QueueEntry
	{
		ErrorType type;
		std::string text;
	};

	static std::vector<QueueEntry> queue;
	static std::vector<std::string> errors;
	static bool error;
	static bool fatalError;
	static bool errorOnWarning;
	static bool silent;
	static int suppressLevel;
};

class TempData
{
public:
	void setFileName(const fs::path& name) { file.setFileName(name); };
	void clear() { file.setFileName({}); }
	void start();
	void end();
	void writeLine(int64_t memoryAddress, const std::string& text);
	bool isOpen() { return file.isOpen(); }
private:
	TextFile file;
};
