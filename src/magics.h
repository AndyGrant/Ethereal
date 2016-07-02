#ifndef _MAGICS_H
#define _MAGICS_H

#include <stdio.h>

void initalizeMagics();
void generateKnightMap();
void generateKingMap();
void generateOccupancyMaskRook();
void generateOccupancyMaskBishop();
void generateOccupancyVariationsRook();
void generateOccupancyVariationsBishop();
void generateMoveDatabaseRook();
void generateMoveDatabaseBishop();

extern int INITIALIZED_MAGICS;

extern uint64_t KnightMap[64];
extern uint64_t KingMap[64];

extern uint64_t OccupancyMaskRook[64];
extern uint64_t OccupancyMaskBishop[64];

extern uint64_t ** OccupancyVariationsRook;
extern uint64_t ** OccupancyVariationsBishop;

extern uint64_t * MoveDatabaseRook;
extern uint64_t * MoveDatabaseBishop;

extern int MagicRookIndexes[64];
extern int MagicBishopIndexes[64];

extern int MagicShiftsRook[64];
extern int MagicShiftsBishop[64];

extern uint64_t MagicNumberRook[64];
extern uint64_t MagicNumberBishop[64];

extern uint8_t * MobilityTableRook;
extern uint8_t * MobilityTableBishop;


#endif