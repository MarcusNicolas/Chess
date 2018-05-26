#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include "Move.h"

struct Entry
{
	Entry() : hash(0), type(PVNode), bestMove(Move()), depth(0), score(0), isAncient(false) {};
	Entry(u64 hash, NodeType type, const Move& bestMove, u8 depth, double score, bool isAncient) : hash(hash), type(type), bestMove(bestMove), depth(depth), score(score), isAncient(isAncient) {};

	u64 hash;
	NodeType type;

	Move bestMove;
	u8 depth;
	double score; // Score for White

	bool isAncient;
};

class TranspositionTable
{
public:
	TranspositionTable(u32);
	~TranspositionTable();

	void tick();

	void addEntry(const Entry&);
	const Entry& getEnty(u64, bool&) const;

private:
	std::vector<Entry*> mTable;
};

#endif // TRANSPOSITIONTABLE_H
