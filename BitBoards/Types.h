#ifndef Types
#define Types

typedef unsigned long long BitBoard;

typedef struct Board {
	BitBoard WhiteAll;
	BitBoard BlackAll;
	BitBoard Kings;
	BitBoard Queens;
	BitBoard Rooks;
	BitBoard Knights;
	BitBoard Bishops;
	BitBoard Pawns;
	
	int Turn;
	int Enpass;
	int FiftyMoveRule;
	
	int ValidCastles[2][2];
	int Castled[2];
	
	BitBoard * KingMap;
	BitBoard * KnightMap;
	BitBoard * OccupancyMaskRook;
	BitBoard * OccupancyMaskBishop;
	int * MagicShiftsRook;
	int * MagicShiftsBishop;
	
	
} Board;

typedef struct Move {
	int Turn;
	int Start;
	int End;
	BitBoard FriendlyBefore;
	BitBoard EnemyBefore;
	int CapturedType;
	int MovedType;
	int Flags;
	
	/* Flags
		0-64 -> pawn double move origin tile
		65 	-> invalidates king-side castle
		66   -> invalidates queen-side castle
		67 	-> Enpass
		68 	-> Promotion
		69	-> Castle
	*/
	
} Move;

#endif