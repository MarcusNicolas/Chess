#ifndef HASHING_H
#define HASHING_H

#include "defs.h"

// Zobrist hashing
class Hashing
{
public:
	static Hashing& instance();

	u64 hashPiece(u8, PieceType, Player) const;
	u64 hashTurn() const;

	u64 hashCastlingRights(u8) const;
	u64 hashEnPassantFile(u8) const;

private:
	Hashing();

	static Hashing mInstance;

	u64 mTurn;

	std::array<std::array<std::array<u64, 64>, 6>, 2> mPieces;
	std::array<u64, 16> mCastlingRights;
	std::array<u64, 8> mEnPassantFile;
};

#endif // HASHING_H
