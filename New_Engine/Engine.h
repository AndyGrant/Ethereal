#ifndef ENGINE_HEADER
#define ENGINE_HEADER

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
	int * TYPES;
	int * COLORS;
	char CHECK_VALIDATIONS[8][8];
	int MOVES_BUFFER[MOVE_SIZE * BUFFER_SIZE];
	int KingLocations[2];
	int Castled[2];
	int ValidCastles[2][2];
	int FiftyMoveRule;
	int * LastMove;
	int KnightMap[64][9];
	int PieceCount[2][6];
} Board;

typedef struct ThreadData{
	Board * board;
	int turn;
	int depth;
	int * lastMove;
	long long added;
} ThreadData;

extern void (*GetPieceMoves[6])(Board *, int, int *, int, int, int);
extern void (*ApplyTypes[5])(Board *, int *);
extern void (*RevertTypes[5])(Board *, int *);

#define ApplyMove(b,m) ((*ApplyTypes[*m])(b,m))
#define RevertMove(b,m) ((*RevertTypes[*m])(b,m))

Board * copyBoard(Board * old);
Board * createBoard(char setup[135]);
void buildKnightMap(Board * board);


void * fooBar(void * ptr);
unsigned long long depthSearch(Board * board, int turn, int depth, int * lastMove);

int * getAllMoves(Board * board, int turn, int * size);
void getPawnMoves(Board * board, int turn, int * size, int x, int y, int check);
void getKnightMoves(Board * board, int turn, int * size, int x, int y, int check);
void getBishopMoves(Board * board, int turn, int * size, int x, int y, int check);
void getRookMoves(Board * board, int turn, int * size, int x, int y, int check);
void getQueenMoves(Board * board, int turn, int * size, int x, int y, int check);
void getKingMoves(Board * board, int turn, int * size, int x, int y, int check);

#define boundsCheck(x,y) ((x >= 0 && x < 8 && y >= 0 && y < 8))

int validateMove(Board * board, int turn);
void pruneCheckValidations(Board * board, int turn);
void fillDirection(Board * board, int turn, int move);

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
