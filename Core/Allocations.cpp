#include "Core/Allocations.h"

#include "Core/Common.h"
#include "Core/Misc.h"

std::map<Allocations::Key, Allocations::Usage> Allocations::allocations;
std::map<Allocations::Key, int64_t> Allocations::pools;
std::multimap<Allocations::Key, Allocations::SubArea> Allocations::subAreas;

void Allocations::clear()
{
	allocations.clear();
}

void Allocations::setArea(int64_t fileID, int64_t position, int64_t space, int64_t usage, bool usesFill, bool shared)
{
	Key key{ fileID, position };
	allocations[key] = Usage{ space, usage, usesFill, shared };
}

void Allocations::forgetArea(int64_t fileID, int64_t position, int64_t space)
{
	Key key{ fileID, position };
	auto it = allocations.find(key);
	if (it != allocations.end() && it->second.space == space)
		allocations.erase(it);

	subAreas.erase(key);
}

void Allocations::setPool(int64_t fileID, int64_t position, int64_t size)
{
	Key key{ fileID, position };
	pools[key] = size;
}

void Allocations::forgetPool(int64_t fileID, int64_t position, int64_t size)
{
	Key key{ fileID, position };
	auto it = pools.find(key);
	if (it != pools.end() && it->second == size)
		pools.erase(it);
}

bool Allocations::allocateSubArea(int64_t fileID, int64_t& position, int64_t minRange, int64_t maxRange, int64_t size)
{
	for (auto it : allocations)
	{
		if (it.first.fileID != fileID || !it.second.shared)
			continue;
		if (minRange != -1 && it.first.position + it.second.space < minRange)
			continue;
		if (maxRange != -1 && it.first.position > maxRange)
			continue;

		int64_t actualUsage = it.second.usage + getSubAreaUsage(it.first);
		int64_t possiblePosition = it.first.position + actualUsage;
		if (minRange != -1 && possiblePosition < minRange)
			continue;
		if (maxRange != -1 && possiblePosition > maxRange)
			continue;

		if (it.second.space >= actualUsage + size)
		{
			position = possiblePosition;
			subAreas.emplace(it.first, SubArea{ actualUsage, size });
			return true;
		}
	}

	return false;
}

void Allocations::clearSubAreas()
{
	subAreas.clear();
}

int64_t Allocations::getSubAreaUsage(int64_t fileID, int64_t position)
{
	Key key{ fileID, position };
	// For safety, just find the end of the last sub area.
	int64_t maxExtent = 0;
	auto range = subAreas.equal_range(key);
	for (auto it = range.first; it != range.second; ++it)
	{
		int64_t extent = it->second.offset + it->second.size;
		maxExtent = std::max(extent, maxExtent);
	}

	// Now subtract out the original usage.
	if (maxExtent <= allocations[key].usage)
		return 0;
	return maxExtent - allocations[key].usage;
}

void Allocations::validateOverlap()
{
	// An easy mistake to make is a "subarea" where the parent area fills, and erases the subarea.
	// Let's detect any sort of area overlap and report a warning.
	Key lastKey{ -1, -1 };
	int64_t lastEndPosition = -1;
	Usage lastUsage{};

	for (auto it : allocations) {
		if (it.first.fileID == lastKey.fileID && it.first.position > lastKey.position && it.first.position < lastEndPosition) {
			// First, the obvious: does the content overlap?
			if (it.first.position < lastKey.position + lastUsage.usage)
				Logger::queueError(Logger::Warning, L"Content of areas %08llX and %08llx overlap", lastKey.position, it.first.position);
			// Next question, does the earlier one fill?
			else if (it.second.usesFill && lastUsage.usesFill)
				Logger::queueError(Logger::Warning, L"Areas %08llX and %08llx overlap and both fill", lastKey.position, it.first.position);

			// If the new area ends before the last, keep it as the last.
			if (lastEndPosition > it.first.position + it.second.space) {
				// But update the usage to the max position.
				int64_t newUsageEnd = it.first.position + it.second.usage + getSubAreaUsage(it.first);
				lastUsage.usage = newUsageEnd - lastKey.position;
				continue;
			}
		}

		lastKey = it.first;
		lastUsage = it.second;
		lastUsage.usage += getSubAreaUsage(lastKey);
		lastEndPosition = it.first.position + it.second.space;
	}
}

AllocationStats Allocations::collectStats()
{
	AllocationStats stats{};
	collectAreaStats(stats);
	collectPoolStats(stats);
	return stats;
}

void Allocations::collectAreaStats(AllocationStats &stats)
{
	// Need to work out overlaps.
	Key lastKey{ -1, -1 };
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

		// We assume overlaps agree on sharing, hopefully...
		if (usage.shared && usage.space - usage.usage > stats.sharedFreeSize - stats.sharedFreeUsage)
		{
			stats.sharedFreePosition = position;
			stats.sharedFreeSize = usage.space;
			stats.sharedFreeUsage = usage.usage;
		}

		stats.totalSize += usage.space;
		stats.totalUsage += usage.usage;
		if (usage.shared)
		{
			stats.sharedSize += usage.space;
			stats.sharedUsage += usage.usage;
		}
	};

	for (auto it : allocations)
	{
		if (it.first.fileID == lastKey.fileID && it.first.position > lastKey.position && it.first.position < lastEndPosition)
		{
			// Overlap, merge.
			int64_t lastUsageEnd = lastKey.position + lastUsage.usage;
			int64_t newUsageEnd = it.first.position + it.second.usage + getSubAreaUsage(it.first);

			if (lastUsageEnd >= it.first.position)
				lastUsage.usage += newUsageEnd - lastUsageEnd;
			else
				lastUsage.usage += it.second.usage + getSubAreaUsage(it.first);

			lastEndPosition = it.first.position + it.second.space;
			lastUsage.space = lastEndPosition - lastKey.position;

			continue;
		}

		if (lastKey.position != -1)
			applyUsage(lastKey.position, lastUsage);

		lastKey = it.first;
		lastUsage = it.second;
		lastUsage.usage += getSubAreaUsage(lastKey);
		lastEndPosition = it.first.position + it.second.space;
	}

	if (lastKey.position != -1)
		applyUsage(lastKey.position, lastUsage);
}

void Allocations::collectPoolStats(AllocationStats &stats)
{
	for (auto it : pools)
	{
		if (it.second > stats.largestPoolSize)
		{
			stats.largestPoolPosition = it.first.position;
			stats.largestPoolSize = it.second;
		}

		stats.totalPoolSize += it.second;
	}
}
