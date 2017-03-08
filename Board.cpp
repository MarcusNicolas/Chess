#include "Board.h"

std::array<Color, 2> Board::mOtherColor = { Black, White };

std::array<u8, 2> Board::mPawnShift = { 8, 64 - 8 };
std::array<i8, 2> Board::mPawnDelta = { 8, -8 };
std::array<u64, 2> Board::mDoublePushMask = { MoveGenerator::rank(Rank3), MoveGenerator::rank(Rank6) };
std::array<u64, 2> Board::mPromotionMask = { MoveGenerator::rank(Rank8), MoveGenerator::rank(Rank1) };

std::array<u8, 2> Board::mCastleDelta    = { 0, 56 };
std::array<u8, 2> Board::mCastleShift    = { 0, 2 };
std::array<u8, 2> Board::mCastleRookFrom = { 7, 0 };
std::array<u8, 2> Board::mCastleRookTo   = { 5, 3 };

Board::Board() :
	mActiveColor(White),
	mHalfmoveClock(0),
	mEnPassantSquare(-1),
	mCastlingRights(WhiteKingCastle | WhiteQueenCastle | BlackKingCastle | BlackQueenCastle),
	mIsGameFinished(false)
{
	// Init bitboards

	mPlayers[White] = 0x000000000000FFFF;
	mPlayers[Black] = 0xFFFF000000000000;

	mOccupancy = mPlayers[White] | mPlayers[Black];


	mPieces[White][Pawn]   = 0x000000000000FF00;
	mPieces[White][Knight] = 0x0000000000000042;
	mPieces[White][Bishop] = 0x0000000000000024;
	mPieces[White][Rook]   = 0x0000000000000081;
	mPieces[White][Queen]  = 0x0000000000000008;
	mPieces[White][King]   = 0x0000000000000010;

	mPieces[Black][Pawn]   = 0x00FF000000000000;
	mPieces[Black][Knight] = 0x4200000000000000;
	mPieces[Black][Bishop] = 0x2400000000000000;
	mPieces[Black][Rook]   = 0x8100000000000000;
	mPieces[Black][Queen]  = 0x0800000000000000;
	mPieces[Black][King]   = 0x1000000000000000;


	_refreshKingSquare(White);
	_refreshKingSquare(Black);

	// Piece type array

	u64 hash(0);
	mPieceTypes.fill(-1);

	for (u8 color(0); color < 2; ++color) {
		for (u8 pieceType(0); pieceType < 6; ++pieceType) {
			u64 p(1);

			for (u8 i(0); i < 64; ++i, p <<= 1) {
				if (~mPieces[color][pieceType] & p)
					continue;

				mPieceTypes[i] = pieceType;
			}
		}
	}

	_generateMoves();

	/*for (u8 i(0); i < 8; ++i) {
		for (u8 j(0); j < 8; ++j)
			std::cout << int(char(mPieceTypes[(7 - i) * 8 + j] + 1)) << " ";

		std::cout << "\n";
	}

	std::cout << "\n\n";*/
}

u64 Board::occupancy() const
{
	return mOccupancy;
}

u64 Board::player(Color color) const
{
	return mPlayers[color];
}

u64 Board::pieces(Color color, PieceType pieceType) const
{
	return mPieces[color][pieceType];
}

const std::list<Move>& Board::possibleMoves() const
{
	return mMoves.top();
}

bool Board::isGameFinished() const
{
	return mIsGameFinished;
}

bool Board::isInCheck(Color color) const
{
	return _isPinned(mKings[color], color);
}

void Board::makeMove(const Move& move)
{
	if (std::find(mMoves.top().begin(), mMoves.top().end(), move) == mMoves.top().end())
		return;

	_makeMove(move);
	_generateMoves();
}

void Board::unmakeMove()
{
	if (!mHistory.size())
		return;

	_unmakeMove();
	mMoves.pop();
}

void Board::_makeMove(const Move& move)
{
	Undo undo({ move, u8(-1), Pawn, mHalfmoveClock, mEnPassantSquare, mCastlingRights });

	++mHalfmoveClock;
	mEnPassantSquare = -1;

	// If castle
	if (move.isCastle()) {
		_movePiece(mCastleDelta[mActiveColor] + mCastleRookFrom[move.type() - 2],
			               mCastleDelta[mActiveColor] + mCastleRookTo[move.type() - 2], 
			               mActiveColor);
	} else {
		// If capture, remove target
		if (move.isCapture()) {
			undo.targetSquare = move.to();

			if (undo.move.type() == EnPassant)
				undo.targetSquare += mPawnDelta[mOtherColor[mActiveColor]];

			undo.targetType = PieceType(mPieceTypes[undo.targetSquare]);

			// If one of the castling rooks is eaten, then remove corresponding castling rights
			if (undo.targetType == Rook && mCastlingRights) {
				switch (char(undo.targetSquare) - mCastleDelta[mOtherColor[mActiveColor]]) {
				case 7:
					mCastlingRights &= ~(WhiteKingCastle << mCastleShift[mOtherColor[mActiveColor]]);
					break;

				case 0:
					mCastlingRights &= ~(WhiteQueenCastle << mCastleShift[mOtherColor[mActiveColor]]);
					break;
				}
			}


			_removePiece(undo.targetSquare, undo.targetType, mOtherColor[mActiveColor]);
			mHalfmoveClock = 0;
		}

		// If promo, add new piece and delete pawn
		if (move.isPromotion()) {
			_removePiece(move.from(), Pawn, mActiveColor);
			_addPiece(move.from(), PieceType((move.type() & 0x3) + 1), mActiveColor);
		}
	}


	// If the king or a rook is moved, then remove castling rights
	if (mCastlingRights) {
		switch (char(move.from()) - mCastleDelta[mActiveColor]) {
		case 4:
			mCastlingRights &= ~(0x3 << mCastleShift[mActiveColor]);
			break;

		case 7:
			mCastlingRights &= ~(WhiteKingCastle << mCastleShift[mActiveColor]);
			break;

		case 0:
			mCastlingRights &= ~(WhiteQueenCastle << mCastleShift[mActiveColor]);
			break;
		}
	}

	// Make the move
	_movePiece(move.from(), move.to(), mActiveColor);

	if (mPieceTypes[move.to()] == Pawn) {
		mHalfmoveClock = 0;

		if (undo.move.type() == DoublePush)
			mEnPassantSquare = move.to() - mPawnDelta[mActiveColor];

	}

	// End the turn
	mActiveColor = mOtherColor[mActiveColor];
	mHistory.push(undo);
}

void Board::_unmakeMove()
{
	Undo undo(mHistory.top());

	mIsGameFinished = false;
	mActiveColor = mOtherColor[mActiveColor];
	mHalfmoveClock = undo.halfmoveClock;
	mEnPassantSquare = undo.enPassantSquare;
	mCastlingRights = undo.castlePerm;

	// Unmake the move
	_movePiece(undo.move.to(), undo.move.from(), mActiveColor);
	
	// Unmake the castle
	if (undo.move.isCastle()) {
		_movePiece(mCastleDelta[mActiveColor] + mCastleRookTo[undo.move.type() - 2], 
			       mCastleDelta[mActiveColor] + mCastleRookFrom[undo.move.type() - 2], 
			       mActiveColor);
	} else {
		// Unmake the promotion
		if (undo.move.isPromotion()) {
			_removePiece(undo.move.from(), PieceType(mPieceTypes[undo.move.from()]), mActiveColor);
			_addPiece(undo.move.from(), Pawn, mActiveColor);
		}

		// Unmake the capture
		if (undo.move.isCapture())
			_addPiece(undo.targetSquare, undo.targetType, mOtherColor[mActiveColor]);
	}

	mHistory.pop();
}

void Board::_movePiece(u8 from, u8 to, Color color)
{
	PieceType pieceType = PieceType(mPieceTypes[from]);

	_removePiece(from, pieceType, color);
	_addPiece(to, pieceType, color);

	_refreshKingSquare(color);
}

void Board::_addPiece(u8 square, PieceType type, Color color)
{
	u64 mask = u64(1) << square;

	mPieces[color][type] |= mask;
	mPlayers[color] |= mask;
	mOccupancy |= mask;

	mPieceTypes[square] = type;
}

void Board::_removePiece(u8 square, PieceType type, Color color)
{
	u64 mask = ~(u64(1) << square);

	mPieces[color][type] &= mask;
	mPlayers[color] &= mask;
	mOccupancy &= mask;

	mPieceTypes[square] = -1;
}

void Board::_refreshKingSquare(Color color)
{
	u64 kings(mPieces[color][King]);
	mKings[color] = bsfReset(kings);
}

void Board::_generateMoves()
{
	mMoves.push(std::list<Move>());

	_addPawnMoves(mActiveColor);
	_addKnightMoves(mActiveColor);
	_addBishopMoves(mActiveColor);
	_addRookMoves(mActiveColor);
	_addQueenMoves(mActiveColor);
	_addKingMoves(mActiveColor);
	
	for (std::list<Move>::iterator it(mMoves.top().begin()); it != mMoves.top().end(); ++it) {
		_makeMove(*it);

        if (isInCheck(mOtherColor[mActiveColor]))
            mMoves.top().erase(it);

		_unmakeMove();
	}

	if (_canCastleKingSide(mActiveColor))
		mMoves.top().push_back(Move(mCastleDelta[mActiveColor] + 4, mCastleDelta[mActiveColor] + 6, KingCastle));

	if (_canCastleQueenSide(mActiveColor))
		mMoves.top().push_back(Move(mCastleDelta[mActiveColor] + 4, mCastleDelta[mActiveColor] + 2, QueenCastle));


	// If no moves are available, then game is finished
	if (!mMoves.size()) {
		mIsGameFinished = true;

		if (isInCheck(mActiveColor))
			mGameIssue = static_cast<GameIssue>(1 - mActiveColor); // Checkmate
		else
			mGameIssue = Draw; // Stalemate
	}
}

void Board::_addPawnMoves(Color color)
{
	u64 pushMoves(0), captureMoves(0),
		pawns(mPieces[color][Pawn]),
		enPassant = u64(1) << mEnPassantSquare;

	if (!pawns)
		return;


	// Pawn push
	pushMoves = circularShift(pawns, mPawnShift[color]) & ~mOccupancy;
	_addMovesDelta(mPawnDelta[color], pushMoves & ~mPromotionMask[color], QuietMove);
	_addPromoDelta(mPawnDelta[color], pushMoves & mPromotionMask[color]);

	// Pawn double push
	pushMoves = circularShift(pushMoves & mDoublePushMask[color], mPawnShift[color]) & ~mOccupancy;
	_addMovesDelta(2 * mPawnDelta[color], pushMoves, DoublePush);



	pushMoves = circularShift(pawns, mPawnShift[color]);

	// Pawn right capture
	captureMoves = circularShift(pushMoves, 1) & ~MoveGenerator::file(FileA);
	_addMovesDelta(mPawnDelta[color] + 1, captureMoves & mPlayers[mOtherColor[color]] & ~mPromotionMask[color], Capture);
	_addMovesDelta(mPawnDelta[color] + 1, captureMoves & enPassant, EnPassant);
	_addPromoCaptureDelta(mPawnDelta[color] + 1, captureMoves & mPlayers[mOtherColor[color]] & mPromotionMask[color]);

	// Pawn left capture
	captureMoves = circularShift(pushMoves, 64 - 1) & ~MoveGenerator::file(FileH);
	_addMovesDelta(mPawnDelta[color] - 1, captureMoves & mPlayers[mOtherColor[color]] & ~mPromotionMask[color], Capture);
	_addMovesDelta(mPawnDelta[color] - 1, captureMoves & enPassant, EnPassant);
	_addPromoCaptureDelta(mPawnDelta[color] - 1, captureMoves & mPlayers[mOtherColor[color]] & mPromotionMask[color]);
}

void Board::_addKnightMoves(Color color)
{
	u8 square(0);
	u64 moves(0), knights(mPieces[color][Knight]);


	while (knights) {
		square = bsfReset(knights);
		moves = MoveGenerator::instance().knightMoves(square);

		_addMovesFrom(square, moves & ~mOccupancy, QuietMove);
		_addMovesFrom(square, moves & mPlayers[mOtherColor[color]], Capture);
	}
}

void Board::_addBishopMoves(Color color)
{
	u8 square(0);
	u64 moves(0), bishops(mPieces[color][Bishop]);

	while (bishops) {
		square = bsfReset(bishops);
		moves = MoveGenerator::instance().bishopMoves(square, mOccupancy);

		_addMovesFrom(square, moves & ~mOccupancy, QuietMove);
		_addMovesFrom(square, moves & mPlayers[mOtherColor[color]], Capture);
	}
}

void Board::_addRookMoves(Color color)
{
	u8 square(0);
	u64 moves(0), rooks(mPieces[color][Rook]);

	while (rooks) {
		square = bsfReset(rooks);
		moves = MoveGenerator::instance().rookMoves(square, mOccupancy);

		_addMovesFrom(square, moves & ~mOccupancy, QuietMove);
		_addMovesFrom(square, moves & mPlayers[mOtherColor[color]], Capture);
	}
}

void Board::_addQueenMoves(Color color)
{
	u8 square(0);
	u64 moves(0), queens(mPieces[color][Queen]);

	while (queens) {
		square = bsfReset(queens);
		moves = MoveGenerator::instance().queenMoves(square, mOccupancy);

		_addMovesFrom(square, moves & ~mOccupancy, QuietMove);
		_addMovesFrom(square, moves & mPlayers[mOtherColor[color]], Capture);
	}
}

void Board::_addKingMoves(Color color)
{
	u64 moves = MoveGenerator::instance().kingMoves(mKings[color]);

	_addMovesFrom(mKings[color], moves & ~mOccupancy, QuietMove);
	_addMovesFrom(mKings[color], moves & mPlayers[mOtherColor[color]], Capture);
}

void Board::_addMovesFrom(u8 from, u64 moves, MoveType type)
{
	u8 to(0);

	while (moves) {
		to = bsfReset(moves);
		mMoves.top().push_back(Move(from, to, type));
	}
}

void Board::_addMovesDelta(i8 delta, u64 moves, MoveType type)
{
	u8 to(0);

	while (moves) {
		to = bsfReset(moves);
		mMoves.top().push_back(Move(to - delta, to, type));
	}
}

void Board::_addPromoDelta(i8 delta, u64 moves)
{
	u8 to(0);

	while (moves) {
		to = bsfReset(moves);
		mMoves.top().push_back(Move(to - delta, to, KnightPromo));
		mMoves.top().push_back(Move(to - delta, to, BishopPromo));
		mMoves.top().push_back(Move(to - delta, to, RookPromo));
		mMoves.top().push_back(Move(to - delta, to, QueenPromo));
	}
}

void Board::_addPromoCaptureDelta(i8 delta, u64 moves)
{
	u8 to(0);

	while (moves) {
		to = bsfReset(moves);
		mMoves.top().push_back(Move(to - delta, to, KnightPromoCapture));
		mMoves.top().push_back(Move(to - delta, to, BishopPromoCapture));
		mMoves.top().push_back(Move(to - delta, to, RookPromoCapture));
		mMoves.top().push_back(Move(to - delta, to, QueenPromoCapture));
	}
}

bool Board::_isPinned(u8 square, Color color) const
{
	Color by(mOtherColor[color]);

	u64 left  = (u64(1) << (square - 1)) & ~MoveGenerator::file(FileH),
        right = (u64(1) << (square + 1)) & ~MoveGenerator::file(FileA);

	return MoveGenerator::instance().rookMoves(square, occupancy()) & (mPieces[by][Rook] | mPieces[by][Queen]) ||
           MoveGenerator::instance().bishopMoves(square, occupancy()) & (mPieces[by][Bishop] | mPieces[by][Queen]) ||
           MoveGenerator::instance().knightMoves(square) & mPieces[by][Knight] ||
           MoveGenerator::instance().kingMoves(square) & mPieces[by][King] ||
           (circularShift(left | right, mPawnShift[color]) & mPieces[by][Pawn]);
}

bool Board::_canCastleKingSide(Color color) const
{
	return (mCastlingRights & (WhiteKingCastle << mCastleShift[color])) &&
		  !(mOccupancy & (u64(0x60) << mCastleDelta[color])) &&
		  !(isInCheck(color) || _isPinned(mCastleDelta[color] + 5, color) || _isPinned(mCastleDelta[color] + 6, color));
}

bool Board::_canCastleQueenSide(Color color) const
{
	return (mCastlingRights & (WhiteQueenCastle << mCastleShift[color])) &&
		  !(mOccupancy & (u64(0xE) << mCastleDelta[color])) &&
		  !(isInCheck(color) || _isPinned(mCastleDelta[color] + 3, color) || _isPinned(mCastleDelta[color] + 2, color));
}
