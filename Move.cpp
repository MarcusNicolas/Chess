#include "Move.h"

Move::Move()
{
}

Move::Move(u8 from, u8 to, MoveType type) :
	mMove(type << 12 | (from & 0x3F) << 6 | (to & 0x3F))
{
}

bool Move::isCastle() const
{
	return (type() & 0xE) == 0x2;
}

bool Move::isCapture() const
{
	return type() & 0x4;;
}

bool Move::isPromotion() const
{
	return type() & 0x8;;
}

u8 Move::from() const
{
	return (mMove >> 6) & 0x3F;
}

u8 Move::to() const
{
	return mMove & 0x3F;
}

MoveType Move::type() const
{
	return MoveType(mMove >> 12);
}

PieceType Move::promotionType() const
{
	return PieceType((type() & 0x3) + 1);
}

bool operator==(const Move& a, const Move& b)
{
	return a.mMove == b.mMove;
}

bool operator<(const Move& a, const Move& b)
{
	return u8(b.isCapture()) < u8(a.isCapture());
}

bool operator>(const Move& a, const Move& b)
{
	return !(a <= b);
}

bool operator<=(const Move& a, const Move& b)
{
	return a < b || a == b;
}

bool operator>=(const Move& a, const Move& b)
{
	return a > b || a == b;
}
