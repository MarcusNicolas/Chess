#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include "Move.h"

struct Result
{
	u64 hash;
	NodeType type;

	Move bestMove;
	u8 depth;
	double score; // Score for White
};

class TranspositionTable
{
public:
	TranspositionTable(u32);
	~TranspositionTable();

	void addEntry(const Result&);
	const Result& getEnty(u64, bool&) const;

private:
	std::vector<Result*> mTable;
};

#endif // TRANSPOSITIONTABLE_H
