#ifndef _ZORBIST_H
#define _ZORBIST_H

#include <stdint.h>

void initalizeZorbist();
uint64_t genRandomBitstring();

static int INITALIZED_ZORBIST = 0;

uint64_t ZorbistKeys[32][64];
uint64_t PawnKeys[32][64];

#endif