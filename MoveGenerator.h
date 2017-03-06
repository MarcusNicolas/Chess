#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include <array>
#include <vector>
#include <list>
#include <map>

#include <random>
#include <chrono>
#include <iostream>

#include "defs.h"

class MoveGenerator
{
	MoveGenerator(const MoveGenerator&) = delete;
	MoveGenerator& operator=(const MoveGenerator&) = delete;

public:
	static MoveGenerator& instance();

	static u64 file(u8);
	static u64 rank(u8);

	u64 knightMoves(u8) const;
	u64 kingMoves(u8) const;

	u64 rookMoves(u8, u64) const;
	u64 bishopMoves(u8, u64) const;
	u64 queenMoves(u8, u64) const;

private:
	MoveGenerator();
	~MoveGenerator();

	std::vector<u64> _permutations(u64);
	void _generateMagics(u8, PieceType);

	u64 _generateRookMoves(u8, u64);
	u64 _generateBishopMoves(u8, u64);

	static MoveGenerator mInstance;

	static std::array<u64, 8> mFiles;
	static std::array<u64, 8> mRanks;

	std::array<u64, 64> mKnightMoves;
	std::array<u64, 64> mKingMoves;

	std::array<std::vector<u64>, 64> mRookMoves;
	std::array<u64, 64> mRookMagics;
	std::array<u64, 64> mRookBlockmasks;

	std::array<std::vector<u64>, 64> mBishopMoves;
	std::array<u64, 64> mBishopMagics;
	std::array<u64, 64> mBishopBlockmasks;
};

#endif // MOVEGENERATOR_H
