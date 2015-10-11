#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "types.h"

void make_normal_move(move_t * m, int from, int to, int cap, int castle){
	*m = 0;
	*m |= from;
	*m |= to << 8;
	*m |= cap << 16;
	*m |= NormalFlag;
	*m |= castle << 28;
}

int main(){
	move_t m;
	make_normal_move(&m, 80, 90, NonePiece, 0);
	
	printf("%d\n",MOVE_IS_NORMAL(m));
	printf("%d\n",MOVE_IS_CASTLE(m));
	printf("%d\n",MOVE_IS_PROMOTION(m));
	printf("%d\n",MOVE_IS_ENPASS(m));
	
	printf("\n");
	
	printf("%d\n",MOVE_GET_FROM(m));
	printf("%d\n",MOVE_GET_TO(m));
	printf("%d\n",MOVE_GET_CAPTURE_TYPE(m));
	printf("%d\n",MOVE_GET_CASTLE_FLAGS(m));
}