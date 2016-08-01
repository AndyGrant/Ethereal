/*
  Ethereal is a UCI chess playing engine authored by Andrew Grant.
  <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>
  
  Ethereal is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  Ethereal is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
