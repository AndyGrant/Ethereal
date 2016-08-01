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

#ifndef _PSQT_H
#define _PSQT_H

void initalizePSQT();

extern int INITALIZED_PSQT;

extern int PSQTopening[32][64];
extern int PSQTendgame[32][64];

extern int InversionTable[64];
extern int PawnOpeningMap32[32];
extern int PawnEndgameMap32[32];
extern int KnightOpeningMap32[32];
extern int KnightEndgameMap32[32];
extern int BishopOpeningMap32[32];
extern int BishopEndgameMap32[32];
extern int RookOpeningMap32[32];
extern int RookEndgameMap32[32];
extern int QueenOpeningMap32[32];
extern int QueenEndgameMap32[32];
extern int KingOpeningMap32[32];
extern int KingEndgameMap32[32];

#endif