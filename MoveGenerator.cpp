#include "MoveGenerator.h"

MoveGenerator MoveGenerator::mInstance;

std::array<u64, 8> MoveGenerator::mFiles = { 0x0101010101010101,
                                             0x0202020202020202,
                                             0x0404040404040404,
                                             0x0808080808080808,
                                             0x1010101010101010,
                                             0x2020202020202020,
                                             0x4040404040404040,
                                             0x8080808080808080 };

std::array<u64, 8> MoveGenerator::mRanks = { 0x00000000000000FF,
                                             0x000000000000FF00,
                                             0x0000000000FF0000,
                                             0x00000000FF000000,
                                             0x000000FF00000000,
                                             0x0000FF0000000000,
                                             0x00FF000000000000,
                                             0xFF00000000000000 };

MoveGenerator& MoveGenerator::instance()
{
	return mInstance;
}

u64 MoveGenerator::file(u8 file)
{
	return mFiles[file];
}

u64 MoveGenerator::rank(u8 rank)
{
	return mRanks[rank];
}

u64 MoveGenerator::knightMoves(u8 square) const
{
	return mKnightMoves[square];
}

u64 MoveGenerator::kingMoves(u8 square) const
{
	return mKingMoves[square];
}

u64 MoveGenerator::rookMoves(u8 square, u64 occupancy) const
{
	return mRookMoves[square][((occupancy & mRookBlockmasks[square]) * mRookMagics[square]) >> (64 - popcount(mRookBlockmasks[square]))];
}

u64 MoveGenerator::bishopMoves(u8 square, u64 occupancy) const
{
	return mBishopMoves[square][((occupancy & mBishopBlockmasks[square]) * mBishopMagics[square]) >> (64 - popcount(mBishopBlockmasks[square]))];
}

u64 MoveGenerator::queenMoves(u8 square, u64 occupancy) const
{
	return rookMoves(square, occupancy) | bishopMoves(square, occupancy);
}

MoveGenerator::MoveGenerator()
{
	mFiles[FileA] = 0x0101010101010101;
	mFiles[FileB] = 0x0202020202020202;
	mFiles[FileC] = 0x0404040404040404;
	mFiles[FileD] = 0x0808080808080808;
	mFiles[FileE] = 0x1010101010101010;
	mFiles[FileF] = 0x2020202020202020;
	mFiles[FileG] = 0x4040404040404040;
	mFiles[FileH] = 0x8080808080808080;

	mRanks[Rank1] = 0x00000000000000FF;
	mRanks[Rank2] = 0x000000000000FF00;
	mRanks[Rank3] = 0x0000000000FF0000;
	mRanks[Rank4] = 0x00000000FF000000;
	mRanks[Rank5] = 0x000000FF00000000;
	mRanks[Rank6] = 0x0000FF0000000000;
	mRanks[Rank7] = 0x00FF000000000000;
	mRanks[Rank8] = 0xFF00000000000000;

	u64 shiftedIndex(0);

	std::cout << "Initializing :\n* Knight moves...";

	// Knight moves
	for (u8 i(0); i < 64; ++i) {
		shiftedIndex = static_cast<u64>(1) << i;

		mKnightMoves[i] = 0;

		mKnightMoves[i] |= (shiftedIndex << 6)  & ~(file(FileG) | file(FileH));
		mKnightMoves[i] |= (shiftedIndex << 15) & ~file(FileH);
		mKnightMoves[i] |= (shiftedIndex << 17) & ~file(FileA);
		mKnightMoves[i] |= (shiftedIndex << 10) & ~(file(FileA) | file(FileB));
		mKnightMoves[i] |= (shiftedIndex >> 6)  & ~(file(FileA) | file(FileB));
		mKnightMoves[i] |= (shiftedIndex >> 15) & ~file(FileA);
		mKnightMoves[i] |= (shiftedIndex >> 17) & ~file(FileH);
		mKnightMoves[i] |= (shiftedIndex >> 10) & ~(file(FileG) | file(FileH));
	}

	std::cout << " done !\n* King moves...";

	// King moves
	for (u8 i(0); i < 64; ++i) {
		shiftedIndex = static_cast<u64>(1) << i;

		mKingMoves[i] = 0;

		mKingMoves[i] |= (shiftedIndex << 7) & ~file(FileH);
		mKingMoves[i] |= (shiftedIndex << 8);
		mKingMoves[i] |= (shiftedIndex << 9) & ~file(FileA);
		mKingMoves[i] |= (shiftedIndex << 1) & ~file(FileA);
		mKingMoves[i] |= (shiftedIndex >> 7) & ~file(FileA);
		mKingMoves[i] |= (shiftedIndex >> 8);
		mKingMoves[i] |= (shiftedIndex >> 9) & ~file(FileH);
		mKingMoves[i] |= (shiftedIndex >> 1) & ~file(FileH);
	}

	std::cout << " done !\n* Rook moves...";

	// Rook blockmasks
	for (u8 r(0); r < 8; ++r) {
		for (u8 f(0); f < 8; ++f) {
			u8 square(r * 8 + f);

			mRookBlockmasks[square] = 0;

			mRookBlockmasks[square] |= file(f) & ~(rank(Rank1) | rank(Rank8));
			mRookBlockmasks[square] |= rank(r) & ~(file(FileA) | file(FileH));
			mRookBlockmasks[square] &= ~(file(f) & rank(r));

			_generateMagics(square, Rook);
		}
	}

	std::cout << " done !\n* Bishop moves...";

	// Bishop blockmasks
	for (u8 i(0); i < 64; ++i) {
		mBishopBlockmasks[i] = _generateBishopMoves(i, 0);
		mBishopBlockmasks[i] &= ~(file(FileA) | file(FileH) | rank(Rank1) | rank(Rank8));

		_generateMagics(i, Bishop);
	}

	std::cout << " done !\n";
}

std::vector<u64> MoveGenerator::_subMasks(u64 mask)
{
	std::vector<u8> bits;
	std::vector<u64> subMasks(pow(2, popcount(mask)));

	u64 shiftedIndex(1);

	for (u8 i(0); i < 64; ++i, shiftedIndex <<= 1) {
		if (mask & shiftedIndex)
			bits.push_back(i);
	}

	for (u16 i(0); i < subMasks.size(); ++i) {
		subMasks[i] = 0;
		shiftedIndex = 1;

		for (u8 p(0); p < bits.size(); ++p, shiftedIndex <<= 1) {
			if (i & shiftedIndex) {
				subMasks[i] |= u64(1) << bits[p];
			}
		}
	}

	return subMasks;
}

void MoveGenerator::_generateMagics(u8 square, PieceType type)
{
	u64& magic(type == Rook ? mRookMagics[square] : mBishopMagics[square]);
	u64 blockmask(type == Rook ? mRookBlockmasks[square] : mBishopBlockmasks[square]);
	std::vector<u64>& movesArray(type == Rook ? mRookMoves[square] : mBishopMoves[square]);

	std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<u64> distribution(0, -1);

	u64 n(0);
	bool fail(true);
	std::vector<u64> subMasks(_subMasks(blockmask));

	while (fail) {
		n = distribution(generator) & distribution(generator) & distribution(generator); // Randomly generate a maybe magic number
		movesArray = std::vector<u64>(subMasks.size(), 0);

		fail = false;

		// Check if the current number is magic
		for (u64 blockers : subMasks) {
			if (fail) break;

			u16 index = (blockers * n) >> (64 - popcount(blockmask));

			if (movesArray[index])
				fail = movesArray[index] != (type == Rook ? _generateRookMoves(square, blockers) : _generateBishopMoves(square, blockers));
			else
				movesArray[index] = (type == Rook ? _generateRookMoves(square, blockers) : _generateBishopMoves(square, blockers));
		}
	}

	magic = n;
}

u64 MoveGenerator::_generateRookMoves(u8 index, u64 occupancy)
{
	u64 moves(0), shiftedIndex(0);


	shiftedIndex = u64(1) << index;

	// Top
	do {
		shiftedIndex <<= 8;
		moves |= shiftedIndex;
	} while (!(occupancy & shiftedIndex) && shiftedIndex);



	shiftedIndex = u64(1) << index;

	// Bottom
	do {
		shiftedIndex >>= 8;
		moves |= shiftedIndex;
	} while (!(occupancy & shiftedIndex) && shiftedIndex);



	shiftedIndex = u64(1) << index;

	// Right
	if (shiftedIndex & ~file(FileH)) {
		do {
			shiftedIndex <<= 1;
			moves |= shiftedIndex;
		} while (!(occupancy & shiftedIndex) && (shiftedIndex & ~file(FileH)));
	}


	shiftedIndex = u64(1) << index;

	// Left
	if (shiftedIndex & ~file(FileA)) {
		do {
			shiftedIndex >>= 1;
			moves |= shiftedIndex;
		} while (!(occupancy & shiftedIndex) && (shiftedIndex & ~file(FileA)));
	}


	shiftedIndex = u64(1) << index;
	moves &= ~shiftedIndex;

	return moves;
}

u64 MoveGenerator::_generateBishopMoves(u8 index, u64 occupancy)
{
	u64 moves(0), shiftedIndex(0);

	shiftedIndex = u64(1) << index;

	if (shiftedIndex & ~file(FileH)) {
		// North East
		do {
			shiftedIndex <<= 9;
			moves |= shiftedIndex;
		} while (!(occupancy & shiftedIndex) && (shiftedIndex & ~file(FileH)));


		shiftedIndex = u64(1) << index;

		// South East
		do {
			shiftedIndex >>= 7;
			moves |= shiftedIndex;
		} while (!(occupancy & shiftedIndex) && (shiftedIndex & ~file(FileH)));
	}


	shiftedIndex = u64(1) << index;

	if (shiftedIndex & ~file(FileA)) {
		// South West
		do {
			shiftedIndex >>= 9;
			moves |= shiftedIndex;
		} while (!(occupancy & shiftedIndex) && (shiftedIndex & ~file(FileA)));


		shiftedIndex = u64(1) << index;

		// North West
		do {
			shiftedIndex <<= 7;
			moves |= shiftedIndex;
		} while (!(occupancy & shiftedIndex) && (shiftedIndex & ~file(FileA)));
	}


	shiftedIndex = u64(1) << index;
	moves &= ~shiftedIndex;

	return moves;
}
