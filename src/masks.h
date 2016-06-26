#ifndef _MASKS_H
#define _MASKS_H

#include <stdint.h>

void initalizeMasks();

extern int INITALIZED_MASKS;

extern uint64_t IsolatedPawnMasks[64];
extern uint64_t PassedPawnMasks[2][64];
extern uint64_t PawnAttackMasks[2][64];
extern uint64_t PawnAdvanceMasks[2][64];
extern uint64_t PawnConnectedMasks[2][64];
extern uint64_t OutpostSquareMasks[2][64];

#endif
