#ifndef _ZORBIST_H
#define _ZORBIST_H

static int INITALIZED_ZORBIST = 0;

#include <stdint.h> // For uint64_t

/* Prototypes */
void init_zorbist();
uint64_t gen_random_bitstring();

uint64_t ZorbistKeys[32][64];

#endif