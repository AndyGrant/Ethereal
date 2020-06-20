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

#pragma once

#include <stdint.h>

#include "types.h"

void initMasks();

int distanceBetween(int sq1, int sq2);
int kingPawnFileDistance(uint64_t pawns, int ksq);
int openFileCount(uint64_t pawns);
uint64_t bitsBetweenMasks(int sq1, int sq2);
uint64_t kingAreaMasks(int colour, int sq);
uint64_t forwardRanksMasks(int colour, int rank);
uint64_t forwardFileMasks(int colour, int sq);
uint64_t adjacentFilesMasks(int file);
uint64_t passedPawnMasks(int colour, int sq);
uint64_t pawnConnectedMasks(int colour, int sq);
uint64_t outpostSquareMasks(int colour, int sq);
uint64_t outpostRanksMasks(int colour);
