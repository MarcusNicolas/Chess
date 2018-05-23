#ifndef AI_H
#define AI_H

#include "Game.h"
#include "TranspositionTable.h"

class AI
{
public:
	AI();

	Move bestMove(const Game&, u8);

private:
	double _evaluate(const Game&, Player) const;
	std::pair<double, Move> _negamax(Game*, u8, double, double, Player);

	TranspositionTable mTranspositionTable;

	std::array<std::array<double, 64>, 6> mPositionalScore;
};

#endif // AI_H
