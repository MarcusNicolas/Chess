#include "TranspositionTable.h"

TranspositionTable::TranspositionTable(size_t size) :
	mTable(size),
	mEntries(0)
{
}

TranspositionTable::~TranspositionTable()
{
	for (Entry* r : mTable)
		if (r != nullptr)
			delete r;
}

size_t TranspositionTable::entries() const
{
	return mEntries;
}

size_t TranspositionTable::size() const
{
	return mTable.size();
}

void TranspositionTable::tick()
{
	for (Entry* r : mTable)
		if (r != nullptr)
			r->isAncient = true;
}

void TranspositionTable::addEntry(const Entry& r)
{
	u32 i(r.hash % mTable.size());

	if (mTable[i] == nullptr) {
		mTable[i] = new Entry(r);
		++mEntries;
	} else if (mTable[i]->isAncient || r.depth > mTable[i]->depth) {
		delete mTable[i];
		mTable[i] = new Entry(r);
	}
}

const Entry& TranspositionTable::getEnty(u64 hash, bool& isEmpty) const
{
	u32 i(hash % mTable.size());
	isEmpty = mTable[i] == nullptr;

	if (isEmpty)
		return Entry();

	mTable[i]->isAncient = false;

	return *mTable[i];

}
