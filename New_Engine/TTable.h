#ifndef TTABLE_HEADER
#define TTABLE_HEADER

#define EXACT = 1
#define LOWERBOUND = 2
#define UPPERBOUND = 3

#define NUM_BUCKETS = 4096
#define BUCKET_SIZE = 512

typedef struct Node{
	int * key;
	int depth;
	int type;
	int turn;
} Node;

typedef struct Bucket{
	int max;
	int size;
	Node * nodes;
} Bucket;

typedef struct TTable{
	int size;
	Bucket * buckets;
} TTable;

Node * createNode(int * key, int depth, int type, int turn);
Bucket * createBucket();
TTable * createTTable();

void getNodeType(Node * node, int alpha, int beta);
Node * getNode(TTable * table, int hash, int * key);
void storeNode(TTable * table, int hash, int * key, int depth, int type, int turn);

int createKey(Board * board);
int createHash(int * key);
 

#endif
