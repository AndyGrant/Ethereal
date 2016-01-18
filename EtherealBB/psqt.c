#include "piece.h"
#include "psqt.h"

void init_psqt(){
	int i, j;
	
	for(i = 0; i < 32; i++){
		for(j = 0; j < 64; j++){
			PSQTopening[i][j] = 0;
			PSQTendgame[i][j] = 0;
		}
	}
	
	for(i = 0, j = 0; i < 64; i += 8, j += 4){
		// Opening Pawn
		PSQTopening[WhitePawn][i+0] = PawnOpeningMap32[j+0];
		PSQTopening[WhitePawn][i+7] = PawnOpeningMap32[j+0];
		PSQTopening[WhitePawn][i+1] = PawnOpeningMap32[j+1];
		PSQTopening[WhitePawn][i+6] = PawnOpeningMap32[j+1];
		PSQTopening[WhitePawn][i+2] = PawnOpeningMap32[j+2];
		PSQTopening[WhitePawn][i+5] = PawnOpeningMap32[j+2];
		PSQTopening[WhitePawn][i+3] = PawnOpeningMap32[j+3];
		PSQTopening[WhitePawn][i+4] = PawnOpeningMap32[j+3];
		                                               
		// Endgame Game                                
		PSQTendgame[WhitePawn][i+0] = PawnEndgameMap32[j+0];
		PSQTendgame[WhitePawn][i+7] = PawnEndgameMap32[j+0];
        PSQTendgame[WhitePawn][i+1] = PawnEndgameMap32[j+1];
        PSQTendgame[WhitePawn][i+6] = PawnEndgameMap32[j+1];
        PSQTendgame[WhitePawn][i+2] = PawnEndgameMap32[j+2];
        PSQTendgame[WhitePawn][i+5] = PawnEndgameMap32[j+2];
        PSQTendgame[WhitePawn][i+3] = PawnEndgameMap32[j+3];
        PSQTendgame[WhitePawn][i+4] = PawnEndgameMap32[j+3];
	}
	
	for(i = BlackPawn; i <= BlackKing; i+= 4){
		for(j = 0; j < 64; j++){
			PSQTopening[i][j] = -PSQTopening[i-1][InversionTable[j]];
			PSQTendgame[i][j] = -PSQTendgame[i-1][InversionTable[j]];
		}
	}
}