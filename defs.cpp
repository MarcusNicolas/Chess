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
