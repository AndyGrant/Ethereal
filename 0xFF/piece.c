#include "piece.h"
#include "assert.h"


int IS_EMPTY_OR_ENEMY(int s, int t){
	if (s == Wall)
		return 0;
	if (s == Empty)
		return 1;
	if ((s%2) != t)
		return 1;
	return 0;
}