#include "Commands/CDirectiveArea.h"

#include "Archs/Architecture.h"
#include "Core/Allocations.h"
#include "Core/Common.h"
#include "Core/FileManager.h"
#include "Core/Misc.h"
#include "Core/SymbolData.h"

#include <algorithm>
#include <cstring>

CDirectiveArea::CDirectiveArea(bool shared, Expression& size)
	: shared(shared), sizeExpression(size), content(nullptr)
{
}

void CDirectiveArea::setFillExpression(Expression& exp)
{
	fillExpression = exp;
}

void CDirectiveArea::setPositionExpression(Expression& exp)
{
	positionExpression = exp;
}

bool CDirectiveArea::Validate(const ValidateState &state)
{
	int64_t oldAreaSize = areaSize;
	int64_t oldContentSize = contentSize;
	int64_t oldPosition = position;

	if (positionExpression.isLoaded())
	{
		if (!positionExpression.evaluateInteger(position))
		{
			Logger::queueError(Logger::Error, "Invalid position expression");
			return false;
		}
		Architecture::current().NextSection();
		g_fileManager->seekVirtual(position);
	}
	else
		position = g_fileManager->getVirtualAddress();

	if (!sizeExpression.evaluateInteger(areaSize))
	{
		Logger::queueError(Logger::Error, "Invalid size expression");
		return false;
	}

	if (areaSize < 0)
	{
		Logger::queueError(Logger::Error, "Negative area size");
		return false;
	}

	if (fillExpression.isLoaded())
	{
		if (!fillExpression.evaluateInteger(fillValue))
		{
			Logger::queueError(Logger::Error, "Invalid fill expression");
			return false;
		}
	}

	bool result = false;
	if (content)
	{
		ValidateState contentValidation = state;
		contentValidation.noFileChange = true;
		contentValidation.noFileChangeDirective = "area";
		content->applyFileInfo();
		result = content->Validate(contentValidation);
	}
	contentSize = g_fileManager->getVirtualAddress()-position;

	// restore info of this command
	applyFileInfo();

	if (areaSize < contentSize)
	{
		Logger::queueError(Logger::Error, "Area at %08x overflowed by %d bytes", position, contentSize - areaSize);
	}

	if (fillExpression.isLoaded() || shared)
		g_fileManager->advanceMemory(areaSize-contentSize);

	if (areaSize != oldAreaSize || contentSize != oldContentSize)
		result = true;

	int64_t oldFileID = fileID;
	fileID = g_fileManager->getOpenFileID();

	if ((oldFileID != fileID || oldPosition != position || areaSize == 0) && oldAreaSize != 0)
		Allocations::forgetArea(oldFileID, oldPosition, oldAreaSize);
	if (areaSize != 0)
		Allocations::setArea(fileID, position, areaSize, contentSize, fillExpression.isLoaded(), shared);

	return result;
}

void CDirectiveArea::Encode() const
{
	if (positionExpression.isLoaded())
	{
		Architecture::current().NextSection();
		g_fileManager->seekVirtual(position);
	}

	if (content)
		content->Encode();

	if (fillExpression.isLoaded())
	{
		int64_t subAreaUsage = Allocations::getSubAreaUsage(fileID, position);
		if (subAreaUsage != 0)
			g_fileManager->advanceMemory(subAreaUsage);

		unsigned char buffer[64];
		memset(buffer,fillValue,64);
		
		size_t writeSize = areaSize-contentSize-subAreaUsage;
		while (writeSize > 0)
		{
			size_t part = std::min<size_t>(64,writeSize);
			g_fileManager->write(buffer,part);
			writeSize -= part;
		}
	}
	else if (shared)
		g_fileManager->advanceMemory(areaSize-contentSize);
}

void CDirectiveArea::writeTempData(TempData& tempData) const
{
	const char *directiveType = shared ? "region" : "area";
	if (positionExpression.isLoaded())
		tempData.writeLine(position, tfm::format(".org 0x%08llX", position));
	if (shared && fillExpression.isLoaded())
		tempData.writeLine(position,tfm::format(".%S 0x%08X,0x%02x",directiveType,areaSize,fillValue));
	else
		tempData.writeLine(position,tfm::format(".%S 0x%08X",directiveType,areaSize));
	if (content)
	{
		content->applyFileInfo();
		content->writeTempData(tempData);
	}

	if (fillExpression.isLoaded() && !shared)
	{
		int64_t subAreaUsage = Allocations::getSubAreaUsage(fileID, position);
		if (subAreaUsage != 0)
			tempData.writeLine(position+contentSize, tfm::format(".skip 0x%08llX",subAreaUsage));

		std::string fillString = tfm::format(".fill 0x%08X,0x%02X",areaSize-contentSize-subAreaUsage,fillValue);
		tempData.writeLine(position+contentSize+subAreaUsage,fillString);
		tempData.writeLine(position+areaSize,tfm::format(".end%S",directiveType));
	} else {
		tempData.writeLine(position+contentSize,tfm::format(".end%S",directiveType));
	}
}

void CDirectiveArea::writeSymData(SymbolData& symData) const
{
	if (content)
		content->writeSymData(symData);

	if (fillExpression.isLoaded())
	{
		int64_t subAreaUsage = Allocations::getSubAreaUsage(fileID, position);
		symData.addData(position+contentSize+subAreaUsage,areaSize-contentSize-subAreaUsage,SymbolData::Data8);
	}
}

CDirectiveAutoRegion::CDirectiveAutoRegion()
{
	this->contentSize = 0;
	this->resetPosition = 0;
	this->position = -1;

	this->content = nullptr;
}

void CDirectiveAutoRegion::setMinRangeExpression(Expression& exp)
{
	minRangeExpression = exp;
}

void CDirectiveAutoRegion::setRangeExpressions(Expression& minExp, Expression& maxExp)
{
	minRangeExpression = minExp;
	maxRangeExpression = maxExp;
}

bool CDirectiveAutoRegion::Validate(const ValidateState &state)
{
	resetPosition = g_fileManager->getVirtualAddress();

	ValidateState contentValidation = state;
	contentValidation.noFileChange = true;
	contentValidation.noFileChangeDirective = "region";

	// We need at least one full pass run before we can get an address.
	if (state.passes < 1)
	{
		// Just calculate contentSize.
		position = g_fileManager->getVirtualAddress();
		content->applyFileInfo();
		content->Validate(contentValidation);
		contentSize = g_fileManager->getVirtualAddress() - position;

		g_fileManager->seekVirtual(resetPosition);
		return true;
	}

	int64_t oldPosition = position;
	int64_t oldContentSize = contentSize;

	int64_t minRange = -1;
	int64_t maxRange = -1;
	if (minRangeExpression.isLoaded())
	{
		if (!minRangeExpression.evaluateInteger(minRange))
		{
			Logger::queueError(Logger::Error, "Invalid range expression for .autoregion");
			return false;
		}
	}
	if (maxRangeExpression.isLoaded())
	{
		if (!maxRangeExpression.evaluateInteger(maxRange))
		{
			Logger::queueError(Logger::Error, "Invalid range expression for .autoregion");
			return false;
		}
	}

	fileID = g_fileManager->getOpenFileID();
	if (!Allocations::allocateSubArea(fileID, position, minRange, maxRange, contentSize))
	{
		Logger::queueError(Logger::Error, "No space available for .autoregion of size %d", contentSize);
		// We might be able to do better next time.
		return Allocations::canTrimSpace();
	}

	Architecture::current().NextSection();
	g_fileManager->seekVirtual(position);

	content->applyFileInfo();
	bool result = content->Validate(contentValidation);
	contentSize = g_fileManager->getVirtualAddress() - position;

	// restore info of this command
	applyFileInfo();
	g_fileManager->seekVirtual(resetPosition);

	if (position != oldPosition || contentSize != oldContentSize)
		result = true;

	return result;
}

void CDirectiveAutoRegion::Encode() const
{
	Architecture::current().NextSection();
	g_fileManager->seekVirtual(position);
	content->Encode();
	g_fileManager->seekVirtual(resetPosition);
}

void CDirectiveAutoRegion::writeTempData(TempData& tempData) const
{
	tempData.writeLine(position,tfm::format(".autoregion 0x%08X",position));
	content->applyFileInfo();
	content->writeTempData(tempData);
	tempData.writeLine(position+contentSize,".endautoregion");
}

void CDirectiveAutoRegion::writeSymData(SymbolData& symData) const
{
	content->writeSymData(symData);
}
