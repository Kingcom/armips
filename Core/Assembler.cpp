#include "Core/Assembler.h"

#include "Archs/ARM/Arm.h"
#include "Archs/ARM/ArmExpressionFunctions.h"
#include "Archs/MIPS/Mips.h"
#include "Archs/MIPS/MipsExpressionFunctions.h"
#include "Archs/SuperH/SuperH.h"
#include "Archs/SuperH/ShExpressionFunctions.h"
#include "Commands/CAssemblerCommand.h"
#include "Core/Allocations.h"
#include "Core/Common.h"
#include "Core/ExpressionFunctions.h"
#include "Core/FileManager.h"
#include "Core/Misc.h"
#include "Core/SymbolData.h"
#include "Parser/Parser.h"

#include <thread>

bool encodeAssembly(std::unique_ptr<CAssemblerCommand> content, SymbolData& symData, TempData& tempData)
{
	bool Revalidate;
	
	Arm.Pass2();
	Mips.Pass2();
	SuperH.Pass2();

	ValidateState validation;
	do	// loop until everything is constant
	{
		Logger::clearQueue();
		Revalidate = false;

		if (validation.passes >= 100)
		{
			Logger::queueError(Logger::Error, "Stuck in infinite validation loop");
			break;
		}

		g_fileManager->reset();
		Allocations::clearSubAreas();

#ifdef _DEBUG
		if (!Logger::isSilent())
			printf("Validate %d...\n",validation.passes);
#endif

		if (Global.memoryMode)
			g_fileManager->openFile(Global.memoryFile,true);

		Revalidate = content->Validate(validation);

		Arm.Revalidate();
		Mips.Revalidate();
		SuperH.Revalidate();

		if (Global.memoryMode)
			g_fileManager->closeFile();

		validation.passes++;
	} while (Revalidate);

	Allocations::validateOverlap();

	Logger::printQueue();
	if (Logger::hasError())
	{
		return false;
	}

#ifdef _DEBUG
	if (!Logger::isSilent())
		printf("Encode...\n");
#endif

	// and finally encode
	if (Global.memoryMode)
		g_fileManager->openFile(Global.memoryFile,false);

	auto writeTempData = [&]()
	{
		tempData.start();
		if (tempData.isOpen())
			content->writeTempData(tempData);
		tempData.end();
	};

	auto writeSymData = [&]()
	{
		content->writeSymData(symData);
		symData.write();
	};

	// writeTempData, writeSymData and encode all access the same
	// memory but never change, so they can run in parallel
	if (Global.multiThreading)
	{
		std::thread tempThread(writeTempData);
		std::thread symThread(writeSymData);

		content->Encode();

		tempThread.join();
		symThread.join();
	} else {
		writeTempData();
		writeSymData();
		content->Encode();
	}

	if (g_fileManager->hasOpenFile())
	{
		if (!Global.memoryMode)
			Logger::printError(Logger::Warning, "File not closed");
		g_fileManager->closeFile();
	}

	if (Logger::hasError())
	{
		return false;
	}

	return true;
}

static void printStats(const AllocationStats &stats)
{
	Logger::printLine("Total areas and regions: %lld / %lld", stats.totalUsage, stats.totalSize);
	Logger::printLine("Total regions: %lld / %lld", stats.sharedUsage, stats.sharedSize);
	Logger::printLine("Largest area or region: 0x%08llX, %lld / %lld", stats.largestPosition, stats.largestUsage, stats.largestSize);

	int64_t startFreePosition = stats.largestFreePosition + stats.largestFreeUsage;
	Logger::printLine("Most free area or region: 0x%08llX, %lld / %lld (free at 0x%08llX)", stats.largestFreePosition, stats.largestFreeUsage, stats.largestFreeSize, startFreePosition);
	int64_t startSharedFreePosition = stats.sharedFreePosition + stats.sharedFreeUsage;
	Logger::printLine("Most free region: 0x%08llX, %lld / %lld (free at 0x%08llX)", stats.sharedFreePosition, stats.sharedFreeUsage, stats.sharedFreeSize, startSharedFreePosition);

	if (stats.totalPoolSize != 0)
	{
		Logger::printLine("Total pool size: %lld", stats.totalPoolSize);
		Logger::printLine("Largest pool: 0x%08llX, %lld", stats.largestPoolPosition, stats.largestPoolSize);
	}
}

bool runArmips(ArmipsArguments& settings)
{
	// initialize and reset global data
	Global.Section = 0;
	Global.nocash = false;
	Global.FileInfo.TotalLineCount = 0;
	Global.relativeInclude = false;
	Global.multiThreading = true;
	Architecture::setCurrent(InvalidArchitecture);

	Tokenizer::clearEquValues();
	Logger::clear();
	Allocations::clear();
	Global.Table.clear();
	Global.symbolTable.clear();

	Global.fileList.clear();
	Global.FileInfo.TotalLineCount = 0;
	Global.FileInfo.LineNumber = 0;
	Global.FileInfo.FileNum = 0;

	Arm.clear();

	ExpressionFunctionHandler::instance().reset();
	registerExpressionFunctions(ExpressionFunctionHandler::instance());

	// process settings
	Parser parser;
	SymbolData symData;
	TempData tempData;
	
	Logger::setSilent(settings.silent);
	Logger::setErrorOnWarning(settings.errorOnWarning);

	if (!settings.symFileName.empty())
		symData.setNocashSymFileName(settings.symFileName, settings.symFileVersion);

	if (!settings.tempFileName.empty())
		tempData.setFileName(settings.tempFileName);

	Token token;
	for (size_t i = 0; i < settings.equList.size(); i++)
	{
		parser.addEquation(token, settings.equList[i].name, settings.equList[i].value);
	}

	Global.symbolTable.addLabels(settings.labels);
	for (const LabelDefinition& label : settings.labels)
	{
		symData.addLabel(label.value, label.name.string());
	}

	if (Logger::hasError())
		return false;

	// run assembler
	TextFile input;
	switch (settings.mode)
	{
	case ArmipsMode::FILE:
		Global.memoryMode = false;		
		if (!input.open(settings.inputFileName,TextFile::Read))
		{
			Logger::printError(Logger::Error, "Could not open file");
			return false;
		}
		break;
	case ArmipsMode::MEMORY:
		Global.memoryMode = true;
		Global.memoryFile = settings.memoryFile;
		input.openMemory(settings.content);
		break;
	}

	std::unique_ptr<CAssemblerCommand> content = parser.parseFile(input);
	Logger::printQueue();

	bool result = !Logger::hasError();
	if (result && content != nullptr)
		result = encodeAssembly(std::move(content), symData, tempData);
	
	if (g_fileManager->hasOpenFile())
	{
		if (!Global.memoryMode)
			Logger::printError(Logger::Warning, "File not closed");
		g_fileManager->closeFile();
	}

	// return errors
	if (settings.errorsResult != nullptr)
	{
		std::vector<std::string> errors = Logger::getErrors();
		for (size_t i = 0; i < errors.size(); i++)
			settings.errorsResult->push_back(errors[i]);
	}

	if (settings.showStats)
		printStats(Allocations::collectStats());

	if (Logger::hasError())
	{
		return false;
	}

	return result;
}
