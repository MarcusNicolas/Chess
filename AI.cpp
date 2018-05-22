#include "AI.h"

std::array<std::array<double, 64>, 6> AI::sPositionalScore =
{   // Pawn
	{ 0., 0., 0., 0., 0., 0., 0., 0.,
	  .05, .10, .10, -.20, -.20, .10, .10, .05,
	  .05, -.05, -.10, 0., 0., -.10, -.05, .05,
	  0., 0., 0., .20, .20, 0., 0., 0.,
	  .05, .05, .10, .25, .25, .10, .05, .05,
	  .10, .10, .20, .30, .30, .20, .10, .10,
	  .50, .50, .50, .50, .50, .50, .50, .50,
	  0., 0., 0., 0., 0., 0., 0., 0.},
	// Knight
	{ -.50, -.40, -.30, -.30, -.30, -.30, -.40, -.50,
	  -.40, -.20, 0., .05, .05, 0., -.20, -.40,
	  -.30, .05, .10, .15, .15, .10, .05, -.30,
	  -.30, 0., .15, .20, .20, .15, 0., -.30,
	  -.30, 0., .15, .20, .20, .15, 0., -.30,
	  -.30, .05, .10, .15, .15, .10, .05, -.30,
	  -.40, -.20, 0., .05, .05, 0., -.20, -.40,
	  -.50, -.40, -.30, -.30, -.30, -.30, -.40, -.50 },
	// Bishop
	{ 0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0. },
	// Rook
	{ 0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0. },
	// Queen
	{ 0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0. },
	// King
	{ 0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0., 
	  0., 0., 0., 0., 0., 0., 0., 0., 
	  0., 0., 0., 0., 0., 0., 0., 0., 
	  0., 0., 0., 0., 0., 0., 0., 0., 
	  0., 0., 0., 0., 0., 0., 0., 0., 
	  0., 0., 0., 0., 0., 0., 0., 0. }
};

AI::AI() :
	mTranspositionTable((u64(1) << 24) + 43) // To make it a prime number
{
}

Move AI::bestMove(const Game& game, u8 depth)
{
	Game root(game);
	return _negamax(&root, depth, -INFINITY, INFINITY, root.activePlayer()).second;
}

double AI::_evaluate(const Game& game, Player player) const
{
	double score = 0;
	
	score += 1.0 * (popcount(game.piecesOf(player, Pawn))   - popcount(game.piecesOf(otherPlayer(player), Pawn)));
	score += 3.2 * (popcount(game.piecesOf(player, Knight)) - popcount(game.piecesOf(otherPlayer(player), Knight)));
	score += 3.3 * (popcount(game.piecesOf(player, Bishop)) - popcount(game.piecesOf(otherPlayer(player), Bishop)));
	score += 5.0 * (popcount(game.piecesOf(player, Rook))   - popcount(game.piecesOf(otherPlayer(player), Rook)));
	score += 9.0 * (popcount(game.piecesOf(player, Queen))  - popcount(game.piecesOf(otherPlayer(player), Queen)));

	switch (game.status())
	{
	case WhiteWin:
		if (player == White)
			score += 1000.;
		else
			score -= 1000.;

		break;

	case BlackWin:
		if (player == Black)
			score += 1000.;
		else
			score -= 1000.;

		break;

	case Draw:
		score -= 200.;
	}

	return score;
}

std::pair<double, Move> AI::_negamax(Game* game, u8 depth, double alpha, double beta, Player player)
{
	if (depth == 0 || game->isOver())
		return std::make_pair(_evaluate(*game, player), Move());

	Move bestMove;
	double score(-INFINITY);

	bool entryIsEmpty(false);
	Result entry;

	entry = mTranspositionTable.getEnty(game->hash(), entryIsEmpty);


	if (!entryIsEmpty && depth <= entry.depth && game->hash() == entry.hash) {
		bestMove = entry.bestMove;
		score = playerSign(player) * entry.score;
	} else {
		double v(0);
		NodeType type(AllNode);

		for (const Move& move : game->possibleMoves()) {
			game->makeMove(move);
			v = -_negamax(game, depth - 1, -beta, -alpha, otherPlayer(player)).first;
			game->unmakeMove();

			if (v > score) {
				score = v;
				bestMove = move;
			}

			if (v > alpha) {
				alpha = v;
				type = PVNode;

				if (alpha > beta) {
					type = CutNode;
					break;
				}
			}
		}

		mTranspositionTable.addEntry({ game->hash(), type, bestMove, depth, playerSign(player) * score });
	}

	return std::make_pair(score, bestMove);
}
