#include "Commands/CDirectiveArea.h"

#include "Archs/Architecture.h"
#include "Core/Allocations.h"
#include "Core/Common.h"
#include "Core/FileManager.h"
#include "Core/Misc.h"
#include "Core/SymbolData.h"

#include <algorithm>
#include <cstring>

CDirectiveArea::CDirectiveArea(Expression& size)
{
	this->areaSize = 0;
	this->contentSize = 0;
	this->position = 0;
	this->fillValue = 0;

	this->sizeExpression = size;
	this->content = nullptr;
}

void CDirectiveArea::setFillExpression(Expression& exp)
{
	fillExpression = exp;
}

bool CDirectiveArea::Validate()
{
	int64_t oldAreaSize = areaSize;
	int64_t oldContentSize = contentSize;
	int64_t oldPosition = position;

	position = g_fileManager->getVirtualAddress();

	if (sizeExpression.evaluateInteger(areaSize) == false)
	{
		Logger::queueError(Logger::Error,L"Invalid size expression");
		return false;
	}

	if (areaSize < 0)
	{
		Logger::queueError(Logger::Error, L"Negative area size");
		return false;
	}

	if (fillExpression.isLoaded())
	{
		if (fillExpression.evaluateInteger(fillValue) == false)
		{
			Logger::queueError(Logger::Error,L"Invalid fill expression");
			return false;
		}
	}

	content->applyFileInfo();
	bool result = content->Validate();
	contentSize = g_fileManager->getVirtualAddress()-position;

	// restore info of this command
	applyFileInfo();

	if (areaSize < contentSize)
	{
		Logger::queueError(Logger::Error, L"Area at %08x overflowed by %d bytes", position, contentSize - areaSize);
	}

	if (fillExpression.isLoaded())
		g_fileManager->advanceMemory(areaSize-contentSize);

	if (areaSize != oldAreaSize || contentSize != oldContentSize)
		result = true;

	int64_t fileID = g_fileManager->getOpenFileID();
	if ((oldPosition != position || areaSize == 0) && oldAreaSize != 0)
		Allocations::forgetArea(fileID, oldPosition, oldAreaSize);
	if (areaSize != 0)
		Allocations::setArea(fileID, position, areaSize, contentSize, fillExpression.isLoaded());

	return result;
}

void CDirectiveArea::Encode() const
{
	content->Encode();

	if (fillExpression.isLoaded())
	{
		int64_t fileID = g_fileManager->getOpenFileID();
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
}

void CDirectiveArea::writeTempData(TempData& tempData) const
{
	tempData.writeLine(position,tfm::format(L".area 0x%08X",areaSize));
	content->applyFileInfo();
	content->writeTempData(tempData);

	if (fillExpression.isLoaded())
	{
		std::wstring fillString = tfm::format(L".fill 0x%08X,0x%02X",areaSize-contentSize,fillValue);
		tempData.writeLine(position+contentSize,fillString);
		tempData.writeLine(position+areaSize,L".endarea");
	} else {
		tempData.writeLine(position+contentSize,L".endarea");
	}
}

void CDirectiveArea::writeSymData(SymbolData& symData) const
{
	content->writeSymData(symData);

	if (fillExpression.isLoaded())
		symData.addData(position+contentSize,areaSize-contentSize,SymbolData::Data8);
}

CDirectiveAutoRegion::CDirectiveAutoRegion()
{
	this->contentSize = 0;
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

bool CDirectiveAutoRegion::Validate()
{
	// We need at least one full pass run before we can get an address.
	if (Global.validationPasses < 1)
	{
		// Just calculate contentSize.
		position = g_fileManager->getVirtualAddress();
		content->applyFileInfo();
		content->Validate();
		contentSize = g_fileManager->getVirtualAddress() - position;
		return true;
	}

	int64_t oldPosition = position;
	int64_t oldContentSize = contentSize;

	int64_t minRange = -1;
	int64_t maxRange = -1;
	if (minRangeExpression.isLoaded())
	{
		if (minRangeExpression.evaluateInteger(minRange) == false)
		{
			Logger::queueError(Logger::Error, L"Invalid range expression for .autoregion");
			return false;
		}
	}
	if (maxRangeExpression.isLoaded())
	{
		if (maxRangeExpression.evaluateInteger(maxRange) == false)
		{
			Logger::queueError(Logger::Error, L"Invalid range expression for .autoregion");
			return false;
		}
	}

	int64_t fileID = g_fileManager->getOpenFileID();
	if (!Allocations::allocateSubArea(fileID, position, minRange, maxRange, contentSize))
	{
		Logger::queueError(Logger::Error, L"No space available for .autoregion of size %d", contentSize);
		return false;
	}

	Arch->NextSection();
	g_fileManager->seekVirtual(position);

	content->applyFileInfo();
	bool result = content->Validate();
	contentSize = g_fileManager->getVirtualAddress() - position;

	// restore info of this command
	applyFileInfo();

	if (position != oldPosition || contentSize != oldContentSize)
		result = true;

	return result;
}

void CDirectiveAutoRegion::Encode() const
{
	Arch->NextSection();
	g_fileManager->seekVirtual(position);
	content->Encode();
}

void CDirectiveAutoRegion::writeTempData(TempData& tempData) const
{
	tempData.writeLine(position,tfm::format(L".autoregion 0x%08X",position));
	content->applyFileInfo();
	content->writeTempData(tempData);
	tempData.writeLine(position+contentSize,L".endautoregion");
}

void CDirectiveAutoRegion::writeSymData(SymbolData& symData) const
{
	content->writeSymData(symData);
}
