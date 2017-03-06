#ifndef MOVE_H
#define MOVE_H

#include "defs.h"

class Move
{
public:
	Move(u8, u8, MoveType);

	bool isCastle() const;
	bool isCapture() const;
	bool isPromotion() const;

	u8 from() const;
	u8 to() const;
	MoveType type() const;

	friend bool operator==(const Move&, const Move&);

private:
	u32 mMove;
};

#endif // MOVE_H
