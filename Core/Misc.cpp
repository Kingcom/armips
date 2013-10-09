#include "stdafx.h"
#include "Misc.h"
#include "Common.h"

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
		QueueError(ERROR_ERROR,"No if clause active");
		return;
	}

	Entry& entry = conditions.back();
	if (entry.isInElseCase)
	{
		QueueError(ERROR_ERROR,"Else case already defined");
		return;
	}

	entry.currentConditionMet = !entry.matchingCaseExecuted;
	entry.isInElseCase = true;
}

void ConditionData::addElseIf(bool conditionMet)
{
	if (conditions.size() == 0)
	{
		QueueError(ERROR_ERROR,"No if clause active");
		return;
	}

	Entry& entry = conditions.back();
	if (entry.isInElseCase)
	{
		QueueError(ERROR_ERROR,"Else case already defined");
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
		QueueError(ERROR_ERROR,"No if clause active");
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


void AreaData::startArea(int start, int size, int fileNum, int lineNumber)
{
	Entry entry;
	entry.start = start;
	entry.maxAddress = start+size;
	entry.fileNum = fileNum;
	entry.lineNumber = lineNumber;
	entry.overflow = false;
	entries.push_back(entry);
}

void AreaData::endArea()
{
	if (entries.size() == 0)
	{
		QueueError(ERROR_ERROR,"No active area");
		return;
	}

	entries.pop_back();
}

bool AreaData::checkAreas(int currentAddress)
{
	bool error = false;

	for (size_t i = 0; i < entries.size(); i++)
	{
		if (entries[i].maxAddress < currentAddress)
		{
			error = true;
			if (entries[i].overflow == false)
			{
				QueueError(ERROR_ERROR,"Area at %s(%d) overflown",
					Global.FileInfo.FileList.GetEntry(entries[i].fileNum),
					entries[i].lineNumber);
				entries[i].overflow = true;
			}
		}
	}

	return error;
}
