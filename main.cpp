#include <iostream>
#include "Board.h"

#include <ctime>

void perft(Board*, u64&, int);
void showBitboard(const u64&);

int main()
{
	Board board;

	std::clock_t start;
	long double duration;
	start = std::clock();

	u64 nodes(0);

	perft(&board, nodes, 6);

	std::cout << nodes << "\n";

	duration = (std::clock() - start) / static_cast<long double>(CLOCKS_PER_SEC);
	std::cout << "Duration: " << duration << "s\n";

	system("PAUSE");
	return 0;
}

void perft(Board* board, u64& nodes, int depth)
{
	std::list<Move> moves(board->possibleMoves());

	if (depth != 1) {
		for (Move move : moves) {
			board->makeMove(move);
			perft(board, nodes, depth - 1);
			board->unmakeMove();
		}
	} else {
		nodes += moves.size();
	}
}

void showBitboard(const u64& bitboard)
{
	u64 p(1);
	p <<= 63;

	for (int i(0); i < 8; ++i) {
		p >>= 7;

		for (int j(0); j < 8; ++j) {
			if (bitboard & p)
				std::cout << "#";
			else
				std::cout << "-";

			if (j != 7) p <<= 1;
		}

		p >>= 8;

		std::cout << "\n";
	}

	std::cout << "\n";
}
