#include "Game.h"

std::array<i8, 2> Game::mPawnShift = { 8, -8 };
std::array<u64, 2> Game::mDoublePushMask = { MoveGenerator::rank(Rank3), MoveGenerator::rank(Rank6) };
std::array<u64, 2> Game::mPromotionMask  = { MoveGenerator::rank(Rank8), MoveGenerator::rank(Rank1) };

std::array<u8, 2> Game::mCastleDelta = { 0, 56 };
std::array<u8, 2> Game::mCastleShift = { 0, 2 };
std::array<u8, 2> Game::mCastleRookFrom = { 7, 0 };
std::array<u8, 2> Game::mCastleRookTo = { 5, 3 };

Game::Game() :
	mActivePlayer(White),
	mHalfmoveClock(0),
	mEnPassantSquare(-1),
	mCastlingRights(WhiteKingCastle | WhiteQueenCastle | BlackKingCastle | BlackQueenCastle),
	mStatus(Ongoing)
{
	// Init bitboards

	mPlayers[White] = 0x000000000000FFFF;
	mPlayers[Black] = 0xFFFF000000000000;

	mOccupancy = mPlayers[White] | mPlayers[Black];


	mPieces[Pawn]   = 0x00FF00000000FF00;
	mPieces[Knight] = 0x4200000000000042;
	mPieces[Bishop] = 0x2400000000000024;
	mPieces[Rook]   = 0x8100000000000081;
	mPieces[Queen]  = 0x0800000000000008;
	mPieces[King]   = 0x1000000000000010;


	_refreshKingSquare(White);
	_refreshKingSquare(Black);

	// Piece type array

	mPieceTypes.fill(-1);

	for (u8 pieceType(0); pieceType < 6; ++pieceType) {
		u64 p(1);

		for (u8 i(0); i < 64; ++i, p <<= 1) {
			if (~mPieces[pieceType] & p)
				continue;

			mPieceTypes[i] = pieceType;
		}
	}

	_generateMoves();

	mHashs.push(0);
}

Game::Game(const Game& game) :
	mOccupancy(game.mOccupancy),
	mPlayers(game.mPlayers),
	mPieces(game.mPieces),
	mKings(game.mKings),
	mPieceTypes(game.mPieceTypes),
	mActivePlayer(game.mActivePlayer),
	mHalfmoveClock(game.mHalfmoveClock),
	mEnPassantSquare(game.mEnPassantSquare),
	mCastlingRights(game.mCastlingRights),
	mStatus(game.mStatus)
{
	mMoves.push(game.possibleMoves());
	mHashs.push(game.hash());
}

Game::~Game()
{
	while (mHistory.size()) {
		if (mHistory.top().targetSquare != nullptr)
			delete mHistory.top().targetSquare;

		mHistory.pop();
	}
}

u64 Game::occupancy() const
{
	return mOccupancy;
}

u64 Game::player(Player player) const
{
	return mPlayers[player];
}

u64 Game::pieces(PieceType pieceType) const
{
	return mPieces[pieceType];
}

u64 Game::piecesOf(Player player, PieceType pieceType) const
{
	return mPlayers[player] & mPieces[pieceType];
}

u8 Game::pieceType(u8 square) const
{
	return mPieceTypes[square];
}

Player Game::activePlayer() const
{
	return mActivePlayer;
}

u8 Game::castlingRights() const
{
	return mCastlingRights;
}

u8 Game::enPassantSquare() const
{
	return mEnPassantSquare;
}

u64 Game::hash() const
{
	return mHashs.top();
}

const std::list<Move>& Game::possibleMoves() const
{
	return mMoves.top();
}

Status Game::status() const
{
	return mStatus;
}

bool Game::isOver() const
{
	return !(status() == Ongoing);
}

bool Game::isKingInCheck(Player player) const
{
	return _isAttacked(mKings[player], player);
}

void Game::makeMove(const Move& move)
{
	if (std::find(mMoves.top().begin(), mMoves.top().end(), move) == mMoves.top().end())
		return;

	mHashs.push(hash() ^ _makeMove(move));
	_generateMoves();


	// If no moves are available, then game is finished
	if (!mMoves.size()) {
		if (isKingInCheck(mActivePlayer))
			mStatus = Status(1 - mActivePlayer); // Checkmate
		else
			mStatus = Draw; // Stalemate
	}

	if (mHalfmoveClock > 100)
		mStatus = Draw;
}

void Game::unmakeMove()
{
	if (!mHistory.size())
		return;

	_unmakeMove();
	mMoves.pop();
	mHashs.pop();

	mStatus = Ongoing;
}

u64 Game::_makeMove(const Move& move)
{
	Undo undo({ move, nullptr, Pawn, mHalfmoveClock, mEnPassantSquare , mCastlingRights });

	++mHalfmoveClock;
	mEnPassantSquare = -1;

	u64 hash(Hashing::instance().hashTurn());
	
	hash ^= Hashing::instance().hashCastlingRights(mCastlingRights);
	hash ^= Hashing::instance().hashEnPassantFile(mEnPassantSquare % 8);
	

	// If castle
	if (move.isCastle()) {
		hash ^= _movePiece(mCastleDelta[mActivePlayer] + mCastleRookFrom[move.type() - 2],
			               mCastleDelta[mActivePlayer] + mCastleRookTo[move.type() - 2],
			               mActivePlayer);
	} else {
		// If capture, remove target
		if (move.isCapture()) {
			undo.targetSquare = new u8(move.to());

			if (move.type() == EnPassant)
				*undo.targetSquare += mPawnShift[otherPlayer(mActivePlayer)];

			undo.targetType = PieceType(mPieceTypes[*undo.targetSquare]);

			// If one of the castling rooks is eaten, then remove corresponding castling rights
			if (undo.targetType == Rook && mCastlingRights) {
				switch (char(*undo.targetSquare) - mCastleDelta[otherPlayer(mActivePlayer)]) {
				case 7:
					mCastlingRights &= ~(WhiteKingCastle << mCastleShift[otherPlayer(mActivePlayer)]);
					break;

				case 0:
					mCastlingRights &= ~(WhiteQueenCastle << mCastleShift[otherPlayer(mActivePlayer)]);
					break;
				}
			}


			hash ^= _removePiece(*undo.targetSquare, undo.targetType, otherPlayer(mActivePlayer));
			mHalfmoveClock = 0;
		}

		// If promo, add new piece and delete pawn
		if (move.isPromotion()) {
			hash ^= _removePiece(move.from(), Pawn, mActivePlayer);
			hash ^= _addPiece(move.from(), move.promotionType(), mActivePlayer);
		}
	}


	// If the king or a rook is moved, then remove castling rights
	if (mCastlingRights) {
		switch (char(move.from()) - mCastleDelta[mActivePlayer]) {
		case 4:
			mCastlingRights &= ~(0x3 << mCastleShift[mActivePlayer]);
			break;

		case 7:
			mCastlingRights &= ~(WhiteKingCastle << mCastleShift[mActivePlayer]);
			break;

		case 0:
			mCastlingRights &= ~(WhiteQueenCastle << mCastleShift[mActivePlayer]);
			break;
		}
	}

	// Make the move
	hash ^= _movePiece(move.from(), move.to(), mActivePlayer);

	if (mPieceTypes[move.to()] == Pawn) {
		mHalfmoveClock = 0;

		if (move.type() == DoublePush)
			mEnPassantSquare = move.to() - mPawnShift[mActivePlayer];

	}


	hash ^= Hashing::instance().hashCastlingRights(mCastlingRights);
	hash ^= Hashing::instance().hashEnPassantFile(mEnPassantSquare % 8);


	// End the turn
	mActivePlayer = otherPlayer(mActivePlayer);
	mHistory.push(undo);

	return hash;
}

void Game::_unmakeMove()
{
	Undo undo(mHistory.top());

	mStatus = Ongoing;
	mActivePlayer = otherPlayer(mActivePlayer);
	mHalfmoveClock = undo.halfmoveClock;
	mEnPassantSquare = undo.enPassantSquare;
	mCastlingRights = undo.castlePerm;

	// Unmake the move
	_movePiece(undo.move.to(), undo.move.from(), mActivePlayer);

	// Unmake the castle
	if (undo.move.isCastle()) {
		_movePiece(mCastleDelta[mActivePlayer] + mCastleRookTo[undo.move.type() - 2],
			mCastleDelta[mActivePlayer] + mCastleRookFrom[undo.move.type() - 2],
			mActivePlayer);
	}
	else {
		// Unmake the promotion
		if (undo.move.isPromotion()) {
			_removePiece(undo.move.from(), PieceType(mPieceTypes[undo.move.from()]), mActivePlayer);
			_addPiece(undo.move.from(), Pawn, mActivePlayer);
		}

		// Unmake the capture
		if (undo.move.isCapture())
			_addPiece(*undo.targetSquare, undo.targetType, otherPlayer(mActivePlayer));
	}

	if (undo.targetSquare != nullptr)
		delete undo.targetSquare;

	mHistory.pop();
}

u64 Game::_movePiece(u8 from, u8 to, Player player)
{
	PieceType pieceType = PieceType(mPieceTypes[from]);

	u64 hash(_removePiece(from, pieceType, player) ^ _addPiece(to, pieceType, player));

	_refreshKingSquare(player);

	return hash;
}

u64 Game::_addPiece(u8 square, PieceType type, Player player)
{
	u64 mask = u64(1) << square;

	mPieces[type] |= mask;
	mPlayers[player] |= mask;
	mOccupancy |= mask;

	mPieceTypes[square] = type;

	return Hashing::instance().hashPiece(square, type, player);
}

u64 Game::_removePiece(u8 square, PieceType type, Player player)
{
	u64 mask = ~(u64(1) << square);

	mPieces[type] &= mask;
	mPlayers[player] &= mask;
	mOccupancy &= mask;

	mPieceTypes[square] = -1;

	return Hashing::instance().hashPiece(square, type, player);
}

void Game::_refreshKingSquare(Player player)
{
	u64 kings(piecesOf(player, King));
	mKings[player] = bsfReset(kings);
}

void Game::_generateMoves()
{
	mMoves.push(std::list<Move>());

	_addPawnMoves(mActivePlayer);
	_addKnightMoves(mActivePlayer);
	_addBishopMoves(mActivePlayer);
	_addRookMoves(mActivePlayer);
	_addQueenMoves(mActivePlayer);
	_addKingMoves(mActivePlayer);

	for (std::list<Move>::iterator it(mMoves.top().begin()); it != mMoves.top().end(); ++it) {
		_makeMove(*it);

		if (isKingInCheck(otherPlayer(mActivePlayer)))
			mMoves.top().erase(it);

		_unmakeMove();
	}

	if (_canCastleKingSide(mActivePlayer))
		mMoves.top().push_back(Move(mCastleDelta[mActivePlayer] + 4, mCastleDelta[mActivePlayer] + 6, KingCastle));

	if (_canCastleQueenSide(mActivePlayer))
		mMoves.top().push_back(Move(mCastleDelta[mActivePlayer] + 4, mCastleDelta[mActivePlayer] + 2, QueenCastle));


	// Sort the generated moves (alpha beta)
	mMoves.top().sort();
}

void Game::_addPawnMoves(Player player)
{
	u64 pushMoves(0), captureMoves(0),
		pawns(piecesOf(player, Pawn)),
		enPassant = u64(1) << mEnPassantSquare;

	if (!pawns)
		return;

	pushMoves = circularShift(pawns, mPawnShift[player]);

	// Pawn right capture
	captureMoves = circularShift(pushMoves, 1) & ~MoveGenerator::file(FileA);
	_addMovesShift(mPawnShift[player] + 1, captureMoves & mPlayers[otherPlayer(player)] & ~mPromotionMask[player], Capture);
	_addMovesShift(mPawnShift[player] + 1, captureMoves & enPassant, EnPassant);
	_addPromoCaptureShift(mPawnShift[player] + 1, captureMoves & mPlayers[otherPlayer(player)] & mPromotionMask[player]);

	// Pawn left capture
	captureMoves = circularShift(pushMoves, 64 - 1) & ~MoveGenerator::file(FileH);
	_addMovesShift(mPawnShift[player] - 1, captureMoves & mPlayers[otherPlayer(player)] & ~mPromotionMask[player], Capture);
	_addMovesShift(mPawnShift[player] - 1, captureMoves & enPassant, EnPassant);
	_addPromoCaptureShift(mPawnShift[player] - 1, captureMoves & mPlayers[otherPlayer(player)] & mPromotionMask[player]);


	// Pawn push
	pushMoves = circularShift(pawns, mPawnShift[player]) & ~mOccupancy;
	_addMovesShift(mPawnShift[player], pushMoves & ~mPromotionMask[player], QuietMove);
	_addPromoShift(mPawnShift[player], pushMoves & mPromotionMask[player]);

	// Pawn double push
	pushMoves = circularShift(pushMoves & mDoublePushMask[player], mPawnShift[player]) & ~mOccupancy;
	_addMovesShift(2 * mPawnShift[player], pushMoves, DoublePush);
}

void Game::_addKnightMoves(Player player)
{
	u8 square(0);
	u64 moves(0), knights(piecesOf(player, Knight));


	while (knights) {
		square = bsfReset(knights);
		moves = MoveGenerator::instance().knightMoves(square);

		_addMovesFrom(square, moves & ~mOccupancy, QuietMove);
		_addMovesFrom(square, moves & mPlayers[otherPlayer(player)], Capture);
	}
}

void Game::_addBishopMoves(Player player)
{
	u8 square(0);
	u64 moves(0), bishops(piecesOf(player, Bishop));

	while (bishops) {
		square = bsfReset(bishops);
		moves = MoveGenerator::instance().bishopMoves(square, mOccupancy);

		_addMovesFrom(square, moves & ~mOccupancy, QuietMove);
		_addMovesFrom(square, moves & mPlayers[otherPlayer(player)], Capture);
	}
}

void Game::_addRookMoves(Player player)
{
	u8 square(0);
	u64 moves(0), rooks(piecesOf(player, Rook));

	while (rooks) {
		square = bsfReset(rooks);
		moves = MoveGenerator::instance().rookMoves(square, mOccupancy);

		_addMovesFrom(square, moves & ~mOccupancy, QuietMove);
		_addMovesFrom(square, moves & mPlayers[otherPlayer(player)], Capture);
	}
}

void Game::_addQueenMoves(Player player)
{
	u8 square(0);
	u64 moves(0), queens(piecesOf(player, Queen));

	while (queens) {
		square = bsfReset(queens);
		moves = MoveGenerator::instance().queenMoves(square, mOccupancy);

		_addMovesFrom(square, moves & ~mOccupancy, QuietMove);
		_addMovesFrom(square, moves & mPlayers[otherPlayer(player)], Capture);
	}
}

void Game::_addKingMoves(Player player)
{
	u64 moves = MoveGenerator::instance().kingMoves(mKings[player]);

	_addMovesFrom(mKings[player], moves & ~mOccupancy, QuietMove);
	_addMovesFrom(mKings[player], moves & mPlayers[otherPlayer(player)], Capture);
}

void Game::_addMovesFrom(u8 from, u64 moves, MoveType type)
{
	u8 to(0);

	while (moves) {
		to = bsfReset(moves);
		mMoves.top().push_back(Move(from, to, type));
	}
}

void Game::_addMovesShift(i8 delta, u64 moves, MoveType type)
{
	u8 to(0);

	while (moves) {
		to = bsfReset(moves);
		mMoves.top().push_back(Move(to - delta, to, type));
	}
}

void Game::_addPromoShift(i8 delta, u64 moves)
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

void Game::_addPromoCaptureShift(i8 delta, u64 moves)
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

bool Game::_isAttacked(u8 square, Player player) const
{
	Player by(otherPlayer(player));

	u64 left = (u64(1) << (square - 1)) & ~MoveGenerator::file(FileH),
		right = (u64(1) << (square + 1)) & ~MoveGenerator::file(FileA);

	return MoveGenerator::instance().rookMoves(square, occupancy()) & (piecesOf(by, Rook) | piecesOf(by, Queen)) ||
		MoveGenerator::instance().bishopMoves(square, occupancy()) & (piecesOf(by, Bishop) | piecesOf(by, Queen)) ||
		MoveGenerator::instance().knightMoves(square) & piecesOf(by, Knight) ||
		MoveGenerator::instance().kingMoves(square) & piecesOf(by, King) ||
		(circularShift(left | right, mPawnShift[player]) & piecesOf(by, Pawn));
}

bool Game::_canCastleKingSide(Player player) const
{
	return (mCastlingRights & (WhiteKingCastle << mCastleShift[player])) &&
		!(mOccupancy & (u64(0x60) << mCastleDelta[player])) &&
		!(isKingInCheck(player) || _isAttacked(mCastleDelta[player] + 5, player) || _isAttacked(mCastleDelta[player] + 6, player));
}

bool Game::_canCastleQueenSide(Player player) const
{
	return (mCastlingRights & (WhiteQueenCastle << mCastleShift[player])) &&
		!(mOccupancy & (u64(0xE) << mCastleDelta[player])) &&
		!(isKingInCheck(player) || _isAttacked(mCastleDelta[player] + 3, player) || _isAttacked(mCastleDelta[player] + 2, player));
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void perft(Game* game, u64& nodes, int depth)
{
	std::list<Move> moves(game->possibleMoves());

	if (depth != 1) {
		for (Move move : moves) {
			game->makeMove(move);
			perft(game, nodes, depth - 1);
			game->unmakeMove();
		}
	}
	else {
		nodes += moves.size();
	}
}
