struct Tuple{
	int value;
	int num;
};

int * findBestMove(struct Board * board, int * last_move, int turn);
int findBestMoveIndex(struct Board * board, int * last_move, int turn);
int alphaBetaPrune(struct Board *b, int turn, int * move, int depth, int alpha, int beta, int evaluatingPlayer);
int evaluateBoard(struct Board *b, int player, int * lastMove);
int evaluateMaterial(struct Board *b, int player);
int evaluateMoves(struct Board *b, int player, int * lastMove);
int * goodHeuristic(struct Board *,int,int *, int);
int * weakHeuristic(struct Board *,int,int *, int);
int evaluate(struct Board *b, int * move, int turn, int value, int depth);