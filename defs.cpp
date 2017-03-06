#include "defs.h"

u8 popCount(u64 x)
{
	return __popcnt64(x);
}

u64 circularShift(u64 x, u8 shift)
{
	return x << shift | x >> (64 - shift);
}

u8 bsfReset(u64& x)
{
	u32 out(0);
	_BitScanForward64(&out, x);

	x &= x - 1;

	return out;
}
