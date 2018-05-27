#ifndef AI_H
#define AI_H

#include "Game.h"
#include "TranspositionTable.h"

class AI
{
public:
	AI(size_t);

	Move bestMove(const Game&, u64);

private:
	static std::array<double, 6> sPiecesValues;

	double _evaluate(const Game&, Player) const;
	std::pair<double, std::list<Move>> _negamax(Game*, u8, double, double, Player, u64&, bool&, const std::chrono::steady_clock::time_point&, u64 duration);
	double _quiescenceSearch(Game*, double, double, Player, u64&);

	TranspositionTable mTranspositionTable;
	std::vector<std::array<Move, 2>> mKillerMoves;

	std::array<std::array<double, 64>, 6> mPositionalScore;
};

#endif // AI_H
