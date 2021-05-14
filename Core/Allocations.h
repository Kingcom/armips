#pragma once

#include <cstdint>
#include <map>

struct AllocationStats
{
	int64_t largestPosition;
	int64_t largestSize;
	int64_t largestUsage;

	int64_t largestFreePosition;
	int64_t largestFreeSize;
	int64_t largestFreeUsage;

	int64_t sharedFreePosition;
	int64_t sharedFreeSize;
	int64_t sharedFreeUsage;

	int64_t totalSize;
	int64_t totalUsage;
	int64_t sharedSize;
	int64_t sharedUsage;

	int64_t largestPoolPosition;
	int64_t largestPoolSize;
	int64_t totalPoolSize;
};

class Allocations
{
public:
	static void clear();
	static void setArea(int64_t fileID, int64_t position, int64_t space, int64_t usage, bool usesFill, bool shared);
	static void forgetArea(int64_t fileID, int64_t position, int64_t space);

	static void setPool(int64_t fileID, int64_t position, int64_t size);
	static void forgetPool(int64_t fileID, int64_t position, int64_t size);

	static void clearSubAreas();
	static bool allocateSubArea(int64_t fileID, int64_t& position, int64_t minRange, int64_t maxRange, int64_t size);
	static int64_t getSubAreaUsage(int64_t fileID, int64_t position);

	static bool canTrimSpace();

	static void validateOverlap();
	static AllocationStats collectStats();

private:
	struct Key
	{
		int64_t fileID;
		int64_t position;

		inline bool operator <(const Allocations::Key &other) const
		{
			return std::tie(fileID, position) < std::tie(other.fileID, other.position);
		}
	};
	struct Usage
	{
		int64_t space;
		int64_t usage;
		bool usesFill;
		bool shared;
	};

	struct SubArea
	{
		int64_t offset;
		int64_t size;
	};

	static void collectAreaStats(AllocationStats &stats);
	static void collectPoolStats(AllocationStats &stats);

	static int64_t getSubAreaUsage(Key key)
	{
		return getSubAreaUsage(key.fileID, key.position);
	}

	static std::map<Key, Usage> allocations;
	static std::map<Key, int64_t> pools;
	static std::multimap<Key, SubArea> subAreas;
	static bool keepPositions;
	static bool nextKeepPositions;
	static bool keptPositions;
};
