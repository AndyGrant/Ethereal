struct Board{
	int types[8][8];
	int colors[8][8];
	int moved[8][8];
	int kingLocations[2];
};

struct Board * createBoard(int[8][8]);
void printBoard(struct Board *);
int * findAllValidMoves(struct Board *, int, int *, int *);
void findPawnMoves(struct Board *, int * , int *, int , int , int, int *);
void findMappedIters(struct Board *, int *, int *, int , int , int , int *, int);
void findMappedNoIters(struct Board *, int *, int *, int , int , int , int *, int);
void findCastles(struct Board *, int * , int *, int , int , int );
void checkMove(struct Board *, int *, int);

