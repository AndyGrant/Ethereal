#ifndef ENGINE_HEADER
#define ENGINE_HEADER

extern int KnightMap[64][9];
extern int KingMap[64][9];

extern const int WHITE;
extern const int BLACK;

extern const int PAWN;
extern const int BISHOP;
extern const int KNIGHT;
extern const int ROOK;
extern const int QUEEN;
extern const int KING;
extern const int EMPTY;

extern const int MOVE_SIZE;

typedef struct Board {
	int Types[8][8];
	int Colors[8][8];
	int KingLocations[2];
	int Castled[2];
	int ValidCastles[2][2];
	int FiftyMoveRule;
} Board;

Board * createBoard(char setup[134]);
int * getAllMoves(Board * board, int turn, int * size, int * previousMoves[8]);

#endif
