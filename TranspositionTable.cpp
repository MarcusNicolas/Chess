#include "TranspositionTable.h"

TranspositionTable::TranspositionTable(u32 size) :
	mTable(size)
{
}

TranspositionTable::~TranspositionTable()
{
	for (Result* r : mTable)
		if (r != nullptr)
			delete r;
}

void TranspositionTable::addEntry(const Result& r)
{
	u32 i(r.hash % mTable.size());

	if (mTable[i] == nullptr || r.depth > mTable[i]->depth)
		mTable[i] = new Result(r);
}

const Result& TranspositionTable::getEnty(u64 hash, bool& isEmpty) const
{
	u32 i(hash % mTable.size());
	isEmpty = mTable[i] == nullptr;

	if (isEmpty)
		return { 0, Move(0, 0, QuietMove), 0, 0 };
	else
		return *mTable[i];
}
