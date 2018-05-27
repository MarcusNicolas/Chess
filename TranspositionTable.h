#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include "Move.h"

struct Entry
{
	Entry() : hash(0), type(PVNode), bestMove(Move()), depth(0), score(0), isAncient(false) {};
	Entry(u64 hash, NodeType type, const Move& bestMove, const std::list<Move>& movesSequence, u8 depth, double score, bool isAncient) : hash(hash), type(type), bestMove(bestMove), movesSequence(movesSequence), depth(depth), score(score), isAncient(isAncient) {};

	u64 hash;
	NodeType type;

	Move bestMove;
	std::list<Move> movesSequence;

	u8 depth;
	double score; // Score for White


	bool isAncient;
};

class TranspositionTable
{
public:
	TranspositionTable(size_t);
	~TranspositionTable();

	size_t entries() const;
	size_t size() const;

	void tick();

	void addEntry(const Entry&);
	const Entry& getEnty(u64, bool&) const;

private:
	std::vector<Entry*> mTable;
	u32 mEntries;
};

#endif // TRANSPOSITIONTABLE_H
