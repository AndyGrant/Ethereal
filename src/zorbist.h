#ifndef _ZORBIST_H
#define _ZORBIST_H

#include <stdint.h>

static int INITALIZED_ZORBIST = 0;

void initalizeZorbist();
uint64_t genRandomBitstring();
uint64_t ZorbistKeys[32][64];

#endif