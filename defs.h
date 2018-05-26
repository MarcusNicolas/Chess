#ifndef DEFS_H
#define DEFS_H

#include <intrin.h>

#include <utility>
#include <algorithm>

#include <array>
#include <vector>
#include <list>
#include <map>
#include <exception>
#include <stack>

#include <chrono>

#include <string>
#include <iostream>

#include <random>
#include <chrono>

#include <SFML\Graphics.hpp>
#include <SFML\Audio.hpp>

typedef char i8;
typedef short i16;
typedef long i32;
typedef long long i64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;

#define INFINITY 1000000.


enum Player {
	White,
	Black
};

enum CastlingFlags {
	WhiteKingCastle = 1,
	WhiteQueenCastle = 2,
	BlackKingCastle = 4,
	BlackQueenCastle = 8
};

enum PieceType {
	Pawn,
	Knight,
	Bishop,
	Rook,
	Queen,
	King
};

enum MoveType {
	QuietMove,
	DoublePush,
	KingCastle,
	QueenCastle,
	Capture,
	EnPassant,
	KnightPromo = 8,
	BishopPromo,
	RookPromo,
	QueenPromo,
	KnightPromoCapture,
	BishopPromoCapture,
	RookPromoCapture,
	QueenPromoCapture
};

enum NodeType {
	PVNode,
	CutNode,
	AllNode
};

enum File {
	FileA,
	FileB,
	FileC,
	FileD,
	FileE,
	FileF,
	FileG,
	FileH
};

enum Rank {
	Rank1,
	Rank2,
	Rank3,
	Rank4,
	Rank5,
	Rank6,
	Rank7,
	Rank8
};

enum Status {
	Ongoing = -1,
	WhiteWin,
	BlackWin,
	Draw
};


u8 popcount(u64);
u64 circularShift(u64, u8);

u8 bsfReset(u64&);

Player otherPlayer(Player);
i8 playerSign(Player);



std::string pieceStr(PieceType);
std::string squareStr(u8);

#endif // DEFS_H
