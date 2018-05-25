#include "defs.h"

u8 popcount(u64 x)
{
	return __popcnt64(x);
}

u64 circularShift(u64 x, u8 shift)
{
	u8 r(shift % 64);
	return x << r | x >> (64 - r);
}

u8 bsfReset(u64& x)
{
	u32 out(0);
	_BitScanForward64(&out, x);

	x &= x - 1;

	return out;
}

Player otherPlayer(Player player)
{
	return Player(1 - player);
}

i8 playerSign(Player player)
{
	return 1 - 2 * player;
}

std::string pieceStr(PieceType type)
{
	switch (type)
	{
	case Pawn:
		return "";
		break;

	case Knight:
		return "N";
		break;

	case Bishop:
		return "B";
		break;

	case Rook:
		return "R";
		break;

	case Queen:
		return "Q";
		break;

	case King:
		return "K";
		break;
	}
}

std::string squareStr(u8 s)
{
	return std::string(1, char('a' + (s%8))) + std::string(1, char('1' + (s/8)));
}
