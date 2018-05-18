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

uint64_t bitsBetweenMasks(int s1, int s2);
uint64_t ranksAtOrAboveMasks(int c, int r);
uint64_t isolatedPawnMasks(int s);
uint64_t passedPawnMasks(int c, int s);
uint64_t pawnConnectedMasks(int c, int s);
uint64_t outpostSquareMasks(int c, int s);
uint64_t outpostRanks(int c);
