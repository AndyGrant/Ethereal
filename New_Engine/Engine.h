#ifndef ENGINE_HEADER
#define ENGINE_HEADER

extern int KnightMap[64][9];
extern int KingMap[64][9];

#define WHITE 0
#define BLACK 1
#define PAWN 0
#define KNIGHT 1
#define BISHOP 2
#define ROOK 3
#define QUEEN 4
#define KING 5
#define EMPTY 9

#define MOVE_SIZE 5
#define BUFFER_SIZE 256

typedef struct Board {
	int Types[8][8];
	int Colors[8][8];
	int KingLocations[2];
	int Castled[2];
	int ValidCastles[2][2];
	int FiftyMoveRule;
	int * PreviousMoves[500];
	int PreviousMovesSize;
} Board;

extern void (*GetPieceMoves[6])(Board *, int, int *, int, int, int);
extern void (*ApplyTypes[5])(Board *, int *);
extern void (*RevertTypes[5])(Board *, int *);

Board * createBoard(char setup[135]);

int * getAllMoves(Board * board, int turn, int * size);
void getPawnMoves(Board * board, int turn, int * size, int x, int y, int check);
void getKnightMoves(Board * board, int turn, int * size, int x, int y, int check);
void getBishopMoves(Board * board, int turn, int * size, int x, int y, int check);
void getRookMoves(Board * board, int turn, int * size, int x, int y, int check);
void getQueenMoves(Board * board, int turn, int * size, int x, int y, int check);
void getKingMoves(Board * board, int turn, int * size, int x, int y, int check);

int validateMove(Board * board, int turn);

void createNormalMove(Board * board, int turn, int * size, int * move, int check);
void createCastleMove(Board * board, int turn, int * size, int * move, int check);
void createPromotionMove(Board * board, int turn, int * size, int * move, int check);
void createEnpassMove(Board * board, int turn, int * size, int * move, int check);

void applyNormalMove(Board * board, int * move);
void applyCastleMove(Board * board, int * move);
void applyPromotionMove(Board * board, int * move);
void applyEnpassMove(Board * board, int * move);

void revertNormalMove(Board * board, int * move);
void revertCastleMove(Board * board, int * move);
void revertPromotionMove(Board * board, int * move);
void revertEnpassMove(Board * board, int * move);




#endif
