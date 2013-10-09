#pragma once
#include <vector>

class ConditionData
{
public:
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
