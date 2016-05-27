#ifndef _MASKS_H
#define _MASKS_H

#include <stdint.h>

void initalizeMasks();

static int INITALIZED_MASKS = 0;

static uint64_t IsolatedPawnMasks[64];
static uint64_t PassedPawnMasks[2][64];
static uint64_t PawnAttackMasks[2][64];
static uint64_t PawnAdvanceMasks[2][64];
static uint64_t PawnConnectedMasks[2][64];
static uint64_t OutpostSquareMasks[2][64];

#endif
