#pragma once

#include <map>

struct AllocationStats {
	int64_t largestPosition;
	int64_t largestSize;
	int64_t largestUsage;

	int64_t largestFreePosition;
	int64_t largestFreeSize;
	int64_t largestFreeUsage;

	int64_t totalSize;
	int64_t totalUsage;
};

class Allocations {
public:
	static void clear();
	static void setArea(int64_t position, int64_t space, int64_t usage);
	static void forgetArea(int64_t position, int64_t space);

	static AllocationStats collectStats();

private:
	struct Usage {
		int64_t space;
		int64_t usage;
	};
	static std::map<int64_t, Usage> allocations;
};
