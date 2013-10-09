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
