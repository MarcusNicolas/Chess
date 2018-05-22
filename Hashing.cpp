#include "Hashing.h"

Hashing Hashing::sInstance;

Hashing& Hashing::instance()
{
	return sInstance;
}

u64 Hashing::hashPiece(u8 square, PieceType type, Player player) const
{
	return mPieces[player][type][square];
}

u64 Hashing::hashTurn() const
{
	return mTurn;
}

u64 Hashing::hashCastlingRights(u8 r) const
{
	return  mCastlingRights[r];
}

u64 Hashing::hashEnPassantFile(u8 file) const
{
	return mEnPassantFile[file];
}

Hashing::Hashing()
{
	std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<u64> distribution(0, -1);

	mTurn = distribution(generator);

	for (u8 player(0); player < mPieces.size(); ++player)
		for (u8 type(0); type < mPieces[player].size(); ++type)
			for (u8 square(0); square < mPieces[player][type].size(); ++square)
				mPieces[player][type][square] = distribution(generator);

	for (u8 i(0); i < mCastlingRights.size(); ++i)
		mCastlingRights[i] = distribution(generator);

	for (u8 i(0); i < mEnPassantFile.size(); ++i)
		mEnPassantFile[i] = distribution(generator);
}
