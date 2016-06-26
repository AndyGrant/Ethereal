#ifndef _PSQT_H
#define _PSQT_H

void initalizePSQT();

extern int INITALIZED_PSQT;

extern int PSQTopening[32][64];
extern int PSQTendgame[32][64];

extern int InversionTable[64];
extern int PawnOpeningMap32[32];
extern int PawnEndgameMap32[32];
extern int KnightOpeningMap32[32];
extern int KnightEndgameMap32[32];
extern int BishopOpeningMap32[32];
extern int BishopEndgameMap32[32];
extern int RookOpeningMap32[32];
extern int RookEndgameMap32[32];
extern int QueenOpeningMap32[32];
extern int QueenEndgameMap32[32];
extern int KingOpeningMap32[32];
extern int KingEndgameMap32[32];

#endif