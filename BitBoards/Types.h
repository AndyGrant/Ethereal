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
	BitBoard Empty;
	BitBoard * Colors[2];
	BitBoard * Pieces[7];
	
	int Turn;
	int Enpass;
	int FiftyMoveRule;
	
	int ValidCastles[2][2];
	int Castled[2];
	
	BitBoard * KingMap;
	BitBoard * KnightMap;
	
	BitBoard * OccupancyMaskRook;
	int * MagicShiftsRook;
	BitBoard * MagicNumberRook;
	BitBoard ** OccupancyVariationsRook;
	BitBoard ** MoveDatabaseRook;
	
	BitBoard * OccupancyMaskBishop;
	int * MagicShiftsBishop;
	BitBoard * MagicNumberBishop;
	BitBoard ** OccupancyVariationsBishop;
	BitBoard ** MoveDatabaseBishop;
	
	
} Board;

typedef struct Move {
	int Start;
	int End;
	int CapturedType;
	int MovedType;
	int Type;
	
	/* Flags
	
		-1		-> No Flags
		0-64 	-> Pawn double move from tile X
		65		-> Promote to Queen
		66		-> Promote to Rook
		67		-> Promote to Knight
		68		-> Promote to Bishop
		70		-> Invalidate Left Castle
		71		-> Invalidate Right Castle
		72		-> Invalidate both Castle		
		80		-> Left Castle
		81		-> Right Castle
		90		-> Enpass
		
	*/
	
} Move;

#endif