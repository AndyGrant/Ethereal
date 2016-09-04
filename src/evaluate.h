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

#ifndef _EVALUATE_H
#define _EVALUATE_H

#include "types.h"

int evaluateBoard(Board * board, PawnTable * ptable);
void evaluatePawns(int * mid, int * end, Board* board, int * pawnCount);
void evaluatePieces(int * mid, int * end, Board * board, int * knightCount, int * bishopCount, int * rookCount, int * queenCount);

#define MG      (0)
#define EG      (1)
#define PhaseNb (2)

#define PawnValue   ( 100)
#define KnightValue ( 325)
#define BishopValue ( 325)
#define RookValue   ( 505)
#define QueenValue  (1000)
#define KingValue   ( 100)

#define KING_HAS_CASTLED     (25)
#define KING_CAN_CASTLE      (10)

#define ROOK_OPEN_FILE_MID   (35)
#define ROOK_OPEN_FILE_END   (20)
#define ROOK_SEMI_FILE_MID   (12)
#define ROOK_SEMI_FILE_END   (12)
#define ROOK_ON_7TH_MID      (10)
#define ROOK_ON_7TH_END      (15)

#define PAWN_STACKED_MID     (10)
#define PAWN_STACKED_END     (20)
#define PAWN_ISOLATED_MID    (10)
#define PAWN_ISOLATED_END    (20)

#define PSQT_MULTIPLIER      (1)

extern int PawnConnected[2][64];
extern int PawnPassedMid[8];
extern int PawnPassedEnd[8];
extern int PieceValues[8];
extern int KnightOutpost[PhaseNb][2];
extern int BishopOutpost[PhaseNb][2];
extern int KnightMobility[PhaseNb][9];
extern int BishopMobility[PhaseNb][14];
extern int RookMobility[PhaseNb][15];
extern int QueenMobility[PhaseNb][28];
extern int SafetyTable[100];
extern int BishopHasWings[PhaseNb];
extern int BishopPair[PhaseNb];

#endif