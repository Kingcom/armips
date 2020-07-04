#include "stdafx.h"
#include "Core/Allocations.h"
#include "Core/Common.h"

std::map<int64_t, Allocations::Usage> Allocations::allocations;

void Allocations::clear()
{
	allocations.clear();
}

void Allocations::setArea(int64_t position, int64_t space, int64_t usage, bool usesFill)
{
	allocations[position] = Usage{ space, usage, usesFill };
}

void Allocations::forgetArea(int64_t position, int64_t space)
{
	auto it = allocations.find(position);
	if (it != allocations.end() && it->second.space == space) {
		allocations.erase(it);
	}
}

void Allocations::validateOverlap()
{
	// An easy mistake to make is a "subarea" where the parent area fills, and erases the subarea.
	// Let's detect any sort of area overlap and report a warning.
	int64_t lastPosition = -1;
	int64_t lastEndPosition = -1;
	Usage lastUsage{};

	for (auto it : allocations) {
		if (it.first > lastPosition && it.first < lastEndPosition) {
			// First, the obvious: does the content overlap?
			if (it.first < lastPosition + lastUsage.usage)
				Logger::queueError(Logger::Warning, L"Content of areas %08llX and %08llx overlap", lastPosition, it.first);
			// Next question, does the earlier one fill?
			else if (it.second.usesFill && lastUsage.usesFill)
				Logger::queueError(Logger::Warning, L"Areas %08llX and %08llx overlap and both fill", lastPosition, it.first);

			// If the new area ends before the last, keep it as the last.
			if (lastEndPosition > it.first + it.second.space) {
				// But update the usage to the max position.
				int64_t newUsageEnd = it.first + it.second.usage;
				lastUsage.usage = newUsageEnd - lastPosition;
				continue;
			}
		}

		lastPosition = it.first;
		lastUsage = it.second;
		lastEndPosition = it.first + it.second.space;
	}
}

AllocationStats Allocations::collectStats()
{
	AllocationStats stats{};

	// Need to work out overlaps.
	int64_t lastPosition = -1;
	int64_t lastEndPosition = -1;
	Usage lastUsage{};

	auto applyUsage = [&stats](int64_t position, const Usage &usage)
	{
		if (usage.space > stats.largestSize)
		{
			stats.largestPosition = position;
			stats.largestSize = usage.space;
			stats.largestUsage = usage.usage;
		}

		if (usage.space - usage.usage > stats.largestFreeSize - stats.largestFreeUsage)
		{
			stats.largestFreePosition = position;
			stats.largestFreeSize = usage.space;
			stats.largestFreeUsage = usage.usage;
		}

		stats.totalSize += usage.space;
		stats.totalUsage += usage.usage;
	};

	for (auto it : allocations)
	{
		if (it.first > lastPosition && it.first < lastEndPosition)
		{
			// Overlap, merge.
			int64_t lastUsageEnd = lastPosition + lastUsage.usage;
			int64_t newUsageEnd = it.first + it.second.usage;

			if (lastUsageEnd >= it.first)
				lastUsage.usage += newUsageEnd - lastUsageEnd;
			else
				lastUsage.usage += it.second.usage;

			lastEndPosition = it.first + it.second.space;
			lastUsage.space = lastEndPosition - lastPosition;

			continue;
		}

		if (lastPosition != -1)
			applyUsage(lastPosition, lastUsage);

		lastPosition = it.first;
		lastUsage = it.second;
		lastEndPosition = it.first + it.second.space;
	}

	if (lastPosition != -1)
		applyUsage(lastPosition, lastUsage);

	return stats;
}
