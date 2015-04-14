#ifndef Engine
#define Engine

extern int PAWN;
extern int PAWN;
extern int KNIGHT;
extern int BISHOP;
extern int ROOK;
extern int QUEEN;
extern int KING;
extern int EMPTY;


typedef struct Board{
	int types[8][8];
	int colors[8][8];
	int moved[8][8];
	int kingLocations[2];
} Board;

Board * createBoard(int board[8][8]);
void printBoard(Board * board);

int * findAllValidMoves(Board * b, int turn, int * moves_size_p, int * last_move);
void findPawnMoves(Board * b, int * moves, int * moves_found, int turn, int x, int y, int * last_move);
void findMappedIters(Board * b, int * moves, int *moves_found, int turn, int x, int y, int * map, int map_size);
void findMappedNoIters(Board * b, int * moves, int * moves_found, int turn, int x, int y, int * map, int map_size);
void findCastles(Board * b, int * moves, int * moves_found, int turn, int x, int y);

void checkMove(Board *b, int * moves_found, int turn);

#endif

