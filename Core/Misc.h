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

class AreaData
{
public:
	void startArea(int start, int size, int fileNum, int lineNumber);
	void endArea();
	bool checkAreas(int currentAddress);
private:
	struct Entry
	{
		int start;
		int maxAddress;
		int fileNum;
		int lineNumber;
		bool overflow;
	};

	std::vector<Entry> entries;
};