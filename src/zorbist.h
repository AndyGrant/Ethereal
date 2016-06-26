#ifndef _ZORBIST_H
#define _ZORBIST_H

#include <stdint.h>

void initalizeZorbist();
uint64_t genRandomBitstring();

extern int INITALIZED_ZORBIST;

extern uint64_t ZorbistKeys[32][64];
extern uint64_t PawnKeys[32][64];

#endif