#ifndef _ZORBIST_H
#define _ZORBIST_H

static int INITALIZED_ZORBIST = 0;

#include <stdint.h> // For uint64_t

/* Prototypes */
void initalizeZorbist();
uint64_t genRandomBitstring();

uint64_t ZorbistKeys[32][64];

#endif