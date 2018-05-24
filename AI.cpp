#include "AI.h"

AI::AI() :
	mTranspositionTable((u64(1) << 24) + 43) // To make it a prime number
{
	mPositionalScore[Pawn] =
		{ 0., 0., 0., 0., 0., 0., 0., 0.,
		  .05, .10, .10, -.20, -.20, .10, .10, .05,
		  .05, -.05, -.10, 0., 0., -.10, -.05, .05,
		  0., 0., 0., .20, .20, 0., 0., 0.,
		  .05, .05, .10, .25, .25, .10, .05, .05,
		  .10, .10, .20, .30, .30, .20, .10, .10,
		  .50, .50, .50, .50, .50, .50, .50, .50,
		  0., 0., 0., 0., 0., 0., 0., 0. };

	mPositionalScore[Knight] =
		{ 0., 0., 0., 0., 0., 0., 0., 0.,
		  .05, .10, .10, -.20, -.20, .10, .10, .05,
		  .05, -.05, -.10, 0., 0., -.10, -.05, .05,
		  0., 0., 0., .20, .20, 0., 0., 0.,
		  .05, .05, .10, .25, .25, .10, .05, .05,
		  .10, .10, .20, .30, .30, .20, .10, .10,
		  .50, .50, .50, .50, .50, .50, .50, .50,
		  0., 0., 0., 0., 0., 0., 0., 0. };

	mPositionalScore[Bishop] =
		{ -.20, -.10, -.10, -.10, -.10, -.10, -.10, -.20,
		  -.10, .05, 0., 0., 0., 0., .05, -.10,
		  -.10, .10, .10, .10, .10, .10, .10, -.10,
		  -.10, 0., .10, .10, 10., .10, 0., -.10,
		  -.10, .05, .05, .10, .10, .05, .05, -.10,
		  -.10, 0., .05, .10, .10, .05, 0., -.10,
		  -.10, 0., 0., 0., 0., 0., 0., -.10,
		  -.20, -.10, -.10, -.10, -.10, -.10, -.10, -.20 };

	mPositionalScore[Rook] =
		{ 0., 0., 0., .05, .05, 0., 0., 0.,
		  -.05, 0., 0., 0., 0., 0., 0., -.05,
		  -.05, 0., 0., 0., 0., 0., 0., -.05,
		  -.05, 0., 0., 0., 0., 0., 0., -.05,
		  -.05, 0., 0., 0., 0., 0., 0., -.05,
		  -.05, 0., 0., 0., 0., 0., 0., -.05,
		  .05, .10, .10, .10, .10, .10, .10, .05,
		  0., 0., 0., 0., 0., 0., 0., 0. };

	mPositionalScore[Queen] =
		{ -.20, -.10, -.10, -.05, -.05, -.10, -.10, -.20,
		  -.10, 0., .05, 0., 0., 0., 0., -.10,
		  -.10, .05, .05, .05, .05, .05, 0., -.10,
		  0., 0., .05, .05, .05, .05, 0., -.05,
		  -.05, 0., .05, .05, .05, .05, 0., -.05,
		  -.10, 0., .05, .05, .05, .05, 0., -.10,
		  -.10, 0., 0., 0., 0., 0., 0., -.10,
		  -.20, -.10, -.10, -.05, -.05, -.10, -.10, -.20 };

	mPositionalScore[King] =
	{ 0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0.,
	  0., 0., 0., 0., 0., 0., 0., 0., 
	  0., 0., 0., 0., 0., 0., 0., 0., 
	  0., 0., 0., 0., 0., 0., 0., 0., 
	  0., 0., 0., 0., 0., 0., 0., 0., 
	  0., 0., 0., 0., 0., 0., 0., 0., 
	  0., 0., 0., 0., 0., 0., 0., 0., };
}

Move AI::bestMove(const Game& game, u8 depth)
{
	Game root(game);
	std::list<Move> l = _negamax(&root, depth, -INFINITY, INFINITY, root.activePlayer()).second;
	
	std::cout << "\n\n";

	for (Move m : l)
		std::cout << "* " << int(m.from()) << " vers " << int(m.to()) << " (" << int(m.type()) << ")\n";

	return l.front();
}

double AI::_evaluate(const Game& game, Player player) const
{
	double score(0.);
	
	score += 1.0 * (popcount(game.piecesOf(player, Pawn))   - popcount(game.piecesOf(otherPlayer(player), Pawn)));
	score += 3.2 * (popcount(game.piecesOf(player, Knight)) - popcount(game.piecesOf(otherPlayer(player), Knight)));
	score += 3.3 * (popcount(game.piecesOf(player, Bishop)) - popcount(game.piecesOf(otherPlayer(player), Bishop)));
	score += 5.0 * (popcount(game.piecesOf(player, Rook))   - popcount(game.piecesOf(otherPlayer(player), Rook)));
	score += 9.0 * (popcount(game.piecesOf(player, Queen))  - popcount(game.piecesOf(otherPlayer(player), Queen)));

	// Bishop pair
	score += 0.3 * ((popcount(game.piecesOf(player, Bishop)) == 2) - (popcount(game.piecesOf(otherPlayer(player), Bishop)) == 2));

	for (u8 i(0); i < 2; ++i) {
		double posScore(0.);

		for (u8 piece(0); piece < 6; ++piece) {
			u64 bitboard(game.piecesOf(Player(i), PieceType(piece)));

			while (bitboard) {
				u8 s = bsfReset(bitboard);
				u8 f(s % 8), r(s / 8);

				if (i == White)
					posScore += mPositionalScore[piece][8*r + f];
				else
					posScore += mPositionalScore[piece][8*(7-r) + f];
			}
		}

		score += posScore * (2 * (i == player) - 1);
	}

	switch (game.status())
	{
	case WhiteWin:
		if (player == White)
			score = INFINITY;
		else
			score = -INFINITY;

		break;

	case BlackWin:
		if (player == Black)
			score = INFINITY;
		else
			score = -INFINITY;

		break;
	}

	return score;
}

std::pair<double, std::list<Move>> AI::_negamax(Game* game, u8 depth, double alpha, double beta, Player player)
{
	/*if (depth == 0 || game->isOver())
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

	return std::make_pair(score, bestMove);*/

	if (depth == 0 || game->isOver())
		return std::make_pair(_evaluate(*game, player), std::list<Move>());

	Move bestMove;
	double score(-INFINITY);

	std::list<Move> l;

	double v(0);

	for (const Move& move : game->possibleMoves()) {
		game->makeMove(move);
		std::pair<double, std::list<Move>> p = _negamax(game, depth - 1, -beta, -alpha, otherPlayer(player));
		v = -p.first;
		game->unmakeMove();

		if (v > score) {
			score = v;
			bestMove = move;
			l = p.second;

			if (score > alpha) {
				alpha = score;
				
				if (alpha >= beta)
					break;
			}
		}
	}

	l.push_front(bestMove);
	return std::make_pair(score, l);
}
