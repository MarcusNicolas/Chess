#include "AI.h"

std::array<double, 6> AI::sPiecesValues = { 1, 3.2, 3.3, 5, 9, 1000 };

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

Move AI::bestMove(const Game& game, u64 thinkingTime)
{
	// Iterative deepening

	u64 nodes(0);
	Game root(game);

	u8 depth(1);
	bool interrupted(false);

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	Move move;
	std::list<Move> movesSequence;


	while (!interrupted) {
		movesSequence = _negamax(&root, depth, -INFINITY, INFINITY, root.activePlayer(), nodes, interrupted, begin, thinkingTime).second;

		if (!interrupted) {
			move = movesSequence.front();
			++depth;
		}
	}

	std::cout << int(double(nodes) / double(thinkingTime)) << " kN/s\n";
	std::cout << int(depth - 1) << "\n\n";

	mTranspositionTable.tick();

	return move;
}

double AI::_evaluate(const Game& game, Player player) const
{
	double score(0);

	// Bishop pair
	score += 0.3 * ((popcount(game.piecesOf(player, Bishop)) == 2) - (popcount(game.piecesOf(otherPlayer(player), Bishop)) == 2));

	for (u8 i(0); i < 2; ++i) {
		double posScore(0.);

		for (u8 piece(0); piece < 6; ++piece) {
			u64 bitboard(game.piecesOf(Player(i), PieceType(piece)));

			posScore += sPiecesValues[piece] * popcount(game.piecesOf(Player(i), PieceType(piece)));

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
			score = 1000.;
		else
			score = -1000.;

		break;

	case BlackWin:
		if (player == Black)
			score = 1000.;
		else
			score = -1000.;

		break;

	case Draw:
		score *= -1;
	}

	return score;
}

std::pair<double, std::list<Move>> AI::_negamax(Game* game, u8 depth, double alpha, double beta, Player player, u64& nodes, bool& interrupted, const std::chrono::steady_clock::time_point& begin, u64 thinkingTime)
{
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

	if (std::chrono::duration_cast<std::chrono::milliseconds>(now - begin).count() > thinkingTime) {
		interrupted = true;
		return std::make_pair(-INFINITY, std::list<Move>());
	}


	++nodes;

	if (depth == 0 || game->isOver())
		return std::make_pair(_quiescenceSearch(game, alpha, beta, player, nodes), std::list<Move>());

	Move bestMove;
	double score(-INFINITY);

	std::list<Move> moveSequence;


	bool entryIsEmpty(false);
	Entry entry;

	bool doSearch(true);

	std::list<Move> possibleMoves(game->possibleMoves());


	entry = mTranspositionTable.getEnty(game->hash(), entryIsEmpty);

	// We matched with an entry in our transposition table
	if (!entryIsEmpty && game->hash() == entry.hash) {
		if (entry.depth >= depth) {
			switch (entry.type)
			{
			case AllNode:
				if (beta > entry.score)
					beta = entry.score;
				break;

			case CutNode:
				if (alpha < entry.score)
					alpha = entry.score;
				break;
			}

			if (entry.type == PVNode || alpha >= beta) {
				bestMove = entry.bestMove;
				score = playerSign(player) * entry.score;
				doSearch = false;
			}
		}

		if (doSearch)
			std::swap_ranges(possibleMoves.begin(), ++possibleMoves.begin(), std::find(possibleMoves.begin(), possibleMoves.end(), entry.bestMove));
	}
	
	if (doSearch) {
		double v(0);
		NodeType type(AllNode);

		for (const Move& move : possibleMoves) {
			game->makeMove(move);
			std::pair<double, std::list<Move>> p = _negamax(game, depth - 1, -beta, -alpha, otherPlayer(player), nodes, interrupted, begin, thinkingTime);
			v = -p.first;
			game->unmakeMove();

			if (v > score) {
				score = v;
				bestMove = move;
				moveSequence = p.second;

				if (score > alpha) {
					alpha = score;
					type = PVNode;

					if (alpha >= beta) {
						type = CutNode;
						break;
					}
				}
			}
		}


		mTranspositionTable.addEntry(Entry(game->hash(), type, bestMove, depth, playerSign(player) * score, false));
	}

	moveSequence.push_front(bestMove);
	return std::make_pair(score, moveSequence); // BUG
}

double AI::_quiescenceSearch(Game* game, double alpha, double beta, Player player, u64& nodes)
{
	double standPat(_evaluate(*game, player));

	if (game->isOver())
		return standPat;

	if (standPat >= beta)
		return beta;

	if (alpha < standPat)
		alpha = standPat;

	double v(0);

	for (const Move& move : game->possibleMoves()) {
		if (!move.isCapture())
			break;

		game->makeMove(move);
		v = -_quiescenceSearch(game, -beta, -alpha, otherPlayer(player), nodes);
		game->unmakeMove();

		if (v >= beta)
			return beta;

		if (v > alpha)
			alpha = v;
	}

	++nodes;

	return alpha;
}
