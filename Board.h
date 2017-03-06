#ifndef BOARD_H
#define BOARD_H

#include <stack>
#include <iostream>

#include <string>

#include "Move.h"
#include "MoveGenerator.h"

struct Undo
{
	Move move;

	u8 targetSquare;
	PieceType targetType;

	u8 halfmoveClock;
	u8 enPassantSquare;
	u8 castlePerm;
};

class Board
{
public:
	Board();

	u64 occupancy() const;

	u64 player(Color) const;
	u64 pieces(Color, PieceType) const;

	const std::list<Move>& possibleMoves() const;

	bool isInCheck(Color) const;

	void makeMove(const Move&);
	void unmakeMove();

private:
	static std::array<Color, 2> mOtherColor;

	static std::array<u8, 2> mPawnShift;
	static std::array<i8, 2> mPawnDelta;
	static std::array<u64, 2> mDoublePushMask;
	static std::array<u64, 2> mPromotionMask;

	static std::array<u8, 2> mCastleDelta;
	static std::array<u8, 2> mCastleShift;
	static std::array<u8, 2> mCastleRookFrom;
	static std::array<u8, 2> mCastleRookTo;


	void _makeMove(const Move&);
	void _unmakeMove();

	void _movePiece(u8, u8, Color);
	void _addPiece(u8, PieceType, Color);
	void _removePiece(u8, PieceType, Color);

	void _refreshKingSquare(Color);

	void _generateMoves();


	void _addPawnMoves(Color);
	void _addKnightMoves(Color);
	void _addBishopMoves(Color);
	void _addRookMoves(Color);
	void _addQueenMoves(Color);
	void _addKingMoves(Color);

	void _addMovesFrom(u8, u64, MoveType);
	void _addMovesDelta(i8, u64, MoveType);
	void _addPromoDelta(i8, u64);
	void _addPromoCaptureDelta(i8, u64);

	bool _isPinned(u8, Color) const;

	bool _canCastleKingSide(Color) const;
	bool _canCastleQueenSide(Color) const;


	u64 mOccupancy;
	std::array<u64, 2> mPlayers;
	std::array<std::array<u64, 6>, 2> mPieces;

	std::array<u8, 2> mKings;

	std::array<u8, 64> mPieceTypes;

	Color mActiveColor;
	u8 mHalfmoveClock;
	u8 mEnPassantSquare;
	u8 mCastlingRights;

	std::stack<std::list<Move>> mMoves;
	std::stack<Undo> mHistory;
};

#endif // BOARD_H
