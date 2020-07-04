#include "stdafx.h"
#include "Allocations.h"

std::map<int64_t, Allocations::Usage> Allocations::allocations;

void Allocations::clear()
{
	allocations.clear();
}

void Allocations::setArea(int64_t position, int64_t space, int64_t usage)
{
	allocations[position] = Usage{ space, usage };
}

void Allocations::forgetArea(int64_t position, int64_t space)
{
	auto it = allocations.find(position);
	if (it != allocations.end() && it->second.space == space) {
		allocations.erase(it);
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
