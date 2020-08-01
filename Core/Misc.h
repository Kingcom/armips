#pragma once

#include "Util/FileClasses.h"

#include <vector>

#include <fmt/format.h>

class Logger
{
public:
	enum ErrorType { Warning, Error, FatalError, Notice };

	static void clear();
	static void print(const std::wstring& text);
	static void printLine(const std::wstring& text);
	static void printLine(const std::string& text);

	template <typename... Args>
	static void print(const wchar_t* text, const Args&... args)
	{
		std::wstring message = fmt::format(text,args...);
		print(message);
	}

	template <typename... Args>
	static void printLine(const wchar_t* text, const Args&... args)
	{
		std::wstring message = fmt::format(text,args...);
		printLine(message);
	}

	template <typename... Args>
	static void printError(ErrorType type, const std::wstring_view& text, const Args&... args)
	{
		// Format message
		fmt::wmemory_buffer buffer;

		formatPreamble(buffer, type);
		fmt::format_to(buffer, text, args...);

		doPrintError(type, fmt::to_string(buffer));
	}

	template <typename... Args>
	static void queueError(ErrorType type, const std::wstring_view& text, const Args&... args)
	{
		// format message
		fmt::wmemory_buffer buffer;

		formatPreamble(buffer, type);
		fmt::format_to(buffer, text, args...);

		doQueueError(type, fmt::to_string(buffer));
	}

	static void printQueue();
	static void clearQueue() { queue.clear(); };
	static std::vector<std::wstring> getErrors() { return errors; };
	static bool hasError() { return error; };
	static bool hasFatalError() { return fatalError; };
	static void setErrorOnWarning(bool b) { errorOnWarning = b; };
	static void setSilent(bool b) { silent = b; };
	static bool isSilent() { return silent; }
	static void suppressErrors() { ++suppressLevel; }
	static void unsuppressErrors() { if (suppressLevel) --suppressLevel; }

private:
	static void formatPreamble(fmt::wmemory_buffer &buffer, ErrorType type);
	static void doPrintError(ErrorType type,std::wstring text);
	static void doQueueError(ErrorType type, std::wstring text);
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
	static int suppressLevel;
};

class TempData
{
public:
	void setFileName(const fs::path& name) { file.setFileName(name); };
	void clear() { file.setFileName({}); }
	void start();
	void end();
	void writeLine(int64_t memoryAddress, const std::wstring_view& text);
	bool isOpen() { return file.isOpen(); }
private:
	TextFile file;
};
