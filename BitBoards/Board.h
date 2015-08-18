#ifndef BoardHeader
#define BoardHeader

#include "Types.h"

// Game Turns
extern int WHITE;
extern int BLACK;

// Piece identifiers
extern int PAWN;
extern int BISHOP;
extern int KNIGHT;
extern int ROOK;
extern int QUEEN;
extern int KING;
extern int EMPTY;

// BitBoards for Inital Board Setup
extern BitBoard startingWhiteAll;
extern BitBoard startingBlackALL;
extern BitBoard startingKings;
extern BitBoard startingQueens;
extern BitBoard startingRooks;
extern BitBoard startingKnights;
extern BitBoard startingBishops;
extern BitBoard startingPawns;

extern BitBoard LeftCastleBoards[2];
extern BitBoard RightCastleBoards[2];

// BitBoards with set bits in Ranks
extern BitBoard RANK_8;
extern BitBoard RANK_7;
extern BitBoard RANK_6;
extern BitBoard RANK_5;
extern BitBoard RANK_4;
extern BitBoard RANK_3;
extern BitBoard RANK_2;
extern BitBoard RANK_1;

extern BitBoard FILE_A;
extern BitBoard FILE_B;
extern BitBoard FILE_C;
extern BitBoard FILE_D;
extern BitBoard FILE_E;
extern BitBoard FILE_F;
extern BitBoard FILE_G;
extern BitBoard FILE_H;

// Functions of Board
Board * BoardInit();
void BoardDeInit(Board * board);

// Build Maps
BitBoard * generateKingMap();
BitBoard * generateKnightMap();
BitBoard * generateOccupancyMaskRook();
BitBoard * generateOccupancyMaskBishop();
BitBoard ** generateOccupancyVariationRook(BitBoard * occupancyMaskRook);
BitBoard ** generateOccupancyVariationBishop(BitBoard * occupancyMaskBishop);
BitBoard ** generateMoveDatabaseRook(Board * board);
BitBoard ** generateMoveDatabaseBishop(Board * board);



#endif