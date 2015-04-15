#ifndef Engine
#define Engine

extern int move_map_knight[8][2];
extern int move_map_bishop[4][2];
extern int move_map_rook[4][2];
extern int move_map_queen[8][2];
extern int move_map_king[8][2];

extern int DIRECTION_MAPPINGS[6][8];

extern int WHITE;
extern int BLACK;


extern int PAWN;
extern int PAWN;
extern int KNIGHT;
extern int BISHOP;
extern int ROOK;
extern int QUEEN;
extern int KING;
extern int EMPTY;

extern int SIZE_OF_MOVE;


typedef struct Board{
	int types[8][8];
	int colors[8][8];
	int moved[8][8];
	int kingLocations[2];
} Board;

Board * createBoard(int board[8][8]);
void printBoard(Board * board);

int * findAllValidMoves(Board * b, int turn, int * moves_size_p, int * last_move);
void findPawnMoves(Board * b, int * moves, int * moves_found, int turn, int x, int y, int * last_move, int eval_check);
void findMappedIters(Board * b, int * moves, int *moves_found, int turn, int x, int y, int * map, int map_size, int eval_check);
void findMappedNoIters(Board * b, int * moves, int * moves_found, int turn, int x, int y, int * map, int map_size, int eval_check);
void findCastles(Board * b, int * moves, int * moves_found, int turn, int x, int y);

void checkMove(Board *b, int * moves_found, int turn);

void determineCheckValidations(int eval_check[8][8], Board * board, int turn);
void checkDirection(int eval_check[8][8], Board * board, int turn, int kx, int ky, int move);
void fillDirection(int eval_check[8][8], int x, int y, int move);

#endif

