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

/* 
    Some Magic Numbers and Magic Shifts, are taken directly
    from a version of the Java engine Rival Chess. The code to
    build the move lookup tables is dervived directly from a blog
    post made by the author of Rival Chess.
    
    Other Magic Numbers are taken from the best of list, found on CPW:
        https://chessprogramming.wikispaces.com/Best+Magics+so+far
*/

#include <stdint.h>
#include <stdlib.h>

#include "bitutils.h"
#include "magics.h"
#include "types.h"

uint64_t KnightMap[SQUARE_NB];
uint64_t KingMap[SQUARE_NB];

uint64_t OccupancyMaskRook[SQUARE_NB];
uint64_t OccupancyMaskBishop[SQUARE_NB];

uint64_t** OccupancyVariationsRook;
uint64_t** OccupancyVariationsBishop;

uint64_t* MoveDatabaseRook;
uint64_t* MoveDatabaseBishop;

int MagicRookIndexes[SQUARE_NB];
int MagicBishopIndexes[SQUARE_NB];

const int MagicShiftsRook[SQUARE_NB] = {
    52,53,53,53,53,53,53,52,
    53,54,54,54,54,54,54,53,
    53,54,54,54,54,54,54,53,
    53,54,54,54,54,54,54,53,
    53,54,54,54,54,54,54,53,
    53,54,54,54,54,54,54,53,
    54,55,55,55,55,55,54,54,
    53,54,54,54,54,53,54,53
};

const int MagicShiftsBishop[SQUARE_NB] = {
    59,60,59,59,59,59,60,59,
    60,60,59,59,59,59,60,60,
    60,60,57,57,57,57,60,60,
    59,59,57,55,55,57,59,59,
    59,59,57,55,55,57,59,59,
    60,60,57,57,57,57,60,60,
    60,60,59,59,59,59,60,60,
    59,60,59,59,59,59,60,59
};

const uint64_t MagicNumberRook[SQUARE_NB] = {
    0xa180022080400230ull, 0x0040100040022000ull, 0x0080088020001002ull, 0x0080080280841000ull,
    0x4200042010460008ull, 0x04800a0003040080ull, 0x0400110082041008ull, 0x008000a041000880ull,
    0x10138001a080c010ull, 0x0000804008200480ull, 0x00010011012000c0ull, 0x0022004128102200ull,
    0x000200081201200cull, 0x202a001048460004ull, 0x0081000100420004ull, 0x4000800380004500ull,
    0x0000208002904001ull, 0x0090004040026008ull, 0x0208808010002001ull, 0x2002020020704940ull,
    0x8048010008110005ull, 0x6820808004002200ull, 0x0a80040008023011ull, 0x00b1460000811044ull,
    0x4204400080008ea0ull, 0xb002400180200184ull, 0x2020200080100380ull, 0x0010080080100080ull,
    0x2204080080800400ull, 0x0000a40080360080ull, 0x02040604002810b1ull, 0x008c218600004104ull,
    0x8180004000402000ull, 0x488c402000401001ull, 0x4018a00080801004ull, 0x1230002105001008ull,
    0x8904800800800400ull, 0x0042000c42003810ull, 0x008408110400b012ull, 0x0018086182000401ull,
    0x2240088020c28000ull, 0x001001201040c004ull, 0x0a02008010420020ull, 0x0010003009010060ull,
    0x0004008008008014ull, 0x0080020004008080ull, 0x0282020001008080ull, 0x50000181204a0004ull,
    0x48fffe99fecfaa00ull, 0x48fffe99fecfaa00ull, 0x497fffadff9c2e00ull, 0x613fffddffce9200ull,
    0xffffffe9ffe7ce00ull, 0xfffffff5fff3e600ull, 0x0010301802830400ull, 0x510ffff5f63c96a0ull,
    0xebffffb9ff9fc526ull, 0x61fffeddfeedaeaeull, 0x53bfffedffdeb1a2ull, 0x127fffb9ffdfb5f6ull,
    0x411fffddffdbf4d6ull, 0x0801000804000603ull, 0x0003ffef27eebe74ull, 0x7645fffecbfea79eull
};

const uint64_t MagicNumberBishop[SQUARE_NB] = {
    0xffedf9fd7cfcffffull, 0xfc0962854a77f576ull, 0x5822022042000000ull, 0x2ca804a100200020ull,
    0x0204042200000900ull, 0x2002121024000002ull, 0xfc0a66c64a7ef576ull, 0x7ffdfdfcbd79ffffull,
    0xfc0846a64a34fff6ull, 0xfc087a874a3cf7f6ull, 0x1001080204002100ull, 0x1810080489021800ull,
    0x0062040420010a00ull, 0x5028043004300020ull, 0xfc0864ae59b4ff76ull, 0x3c0860af4b35ff76ull,
    0x73c01af56cf4cffbull, 0x41a01cfad64aaffcull, 0x040c0422080a0598ull, 0x4228020082004050ull,
    0x0200800400e00100ull, 0x020b001230021040ull, 0x7c0c028f5b34ff76ull, 0xfc0a028e5ab4df76ull,
    0x0020208050a42180ull, 0x001004804b280200ull, 0x2048020024040010ull, 0x0102c04004010200ull,
    0x020408204c002010ull, 0x02411100020080c1ull, 0x102a008084042100ull, 0x0941030000a09846ull,
    0x0244100800400200ull, 0x4000901010080696ull, 0x0000280404180020ull, 0x0800042008240100ull,
    0x0220008400088020ull, 0x04020182000904c9ull, 0x0023010400020600ull, 0x0041040020110302ull,
    0xdcefd9b54bfcc09full, 0xf95ffa765afd602bull, 0x1401210240484800ull, 0x0022244208010080ull,
    0x1105040104000210ull, 0x2040088800c40081ull, 0x43ff9a5cf4ca0c01ull, 0x4bffcd8e7c587601ull,
    0xfc0ff2865334f576ull, 0xfc0bf6ce5924f576ull, 0x80000b0401040402ull, 0x0020004821880a00ull,
    0x8200002022440100ull, 0x0009431801010068ull, 0xc3ffb7dc36ca8c89ull, 0xc3ff8a54f4ca2c89ull,
    0xfffffcfcfd79edffull, 0xfc0863fccb147576ull, 0x040c000022013020ull, 0x2000104000420600ull,
    0x0400000260142410ull, 0x0800633408100500ull, 0xfc087e8e4bb2f736ull, 0x43ff9e4ef4ca2c89ull
};

void initializeMagics(){
    
    int i;
    uint64_t j;
    
    generateKnightMap();
    generateKingMap();
    generateRookIndexes();
    generateBishopIndexes();
    generateOccupancyMaskRook();
    generateOccupancyMaskBishop();
    generateOccupancyVariationsRook();
    generateOccupancyVariationsBishop();
    generateMoveDatabaseRook();
    generateMoveDatabaseBishop();
    
    // Clean up occupancy variations for bishops
    for (i = 0; i < SQUARE_NB; i++)
        free(OccupancyVariationsBishop[i]);
    free(OccupancyVariationsBishop);
    
    // Clean up occupancy variations for rooks
    for (i = 0; i < SQUARE_NB; i++)
        free(OccupancyVariationsRook[i]);
    free(OccupancyVariationsRook);
    
    // Initalize BitCounts for popcount()
    for (j = 0ull; j < 0x10000ull; j++)
        BitCounts[j] = countSetBits(j);
}

void generateKnightMap(){
    
    int i;
    
    for(i = 0; i < SQUARE_NB; i++){
        
        // Up and to the Right
        if (i + 17 < SQUARE_NB && i % 8 != 7)
            KnightMap[i] |= 1ull << (i + 17);
        
        // Down and to the Left
        if (i - 17 >= 0 && i % 8 != 0)
            KnightMap[i] |= 1ull << (i - 17);
        
        // Up and to the Left
        if (i + 15 < SQUARE_NB && i % 8 != 0)
            KnightMap[i] |= 1ull << (i + 15);
        
        // Down and to the Right
        if (i - 15 >= 0 && i % 8 != 7)
            KnightMap[i] |= 1ull << (i - 15);
        
        // To the Right and Up
        if (i + 10 < SQUARE_NB && i % 8 <= 5)
            KnightMap[i] |= 1ull << (i + 10);
        
        // To the Left and Down
        if (i - 10 >= 0 && i % 8 >= 2)
            KnightMap[i] |= 1ull << (i - 10);
        
        // To the Left and Up
        if (i + 6  < SQUARE_NB && i % 8 >= 2)
            KnightMap[i] |= 1ull << (i + 6);
        
        // To the Right and Down
        if (i - 6  >= 0 && i % 8 <= 5)
            KnightMap[i] |= 1ull << (i - 6);
    }
}

void generateRookIndexes(){
    
    int i, sum;
    
    for (i = 0, sum = 0; i < SQUARE_NB; i++){
        MagicRookIndexes[i] = sum;
        sum += (1 << (64 - MagicShiftsRook[i]));
    }
}

void generateBishopIndexes(){
    
    int i, sum;
    
    for (i = 0, sum = 0; i < SQUARE_NB; i++){
        MagicBishopIndexes[i] = sum;
        sum += (1 << (64 - MagicShiftsBishop[i]));
    }
}

void generateKingMap(){
    
    int i;
    
    for(i = 0; i < SQUARE_NB; i++){
        
        // Up and to the Right
        if (i + 9 < SQUARE_NB && i % 8 != 7)
            KingMap[i] |= 1ull << (i + 9);
        
        // Down and to the Left
        if (i - 9 >= 0 && i % 8 != 0)
            KingMap[i] |= 1ull << (i - 9); 
        
        // Up and the the Left
        if (i + 7 < SQUARE_NB && i % 8 != 0)
            KingMap[i] |= 1ull << (i + 7);
        
        // Down and to the Right
        if (i - 7 >= 0 && i % 8 != 7)
            KingMap[i] |= 1ull << (i - 7); 
        
        // To the Right
        if (i + 1 < SQUARE_NB && i % 8 != 7)
            KingMap[i] |= 1ull << (i + 1);
        
        // To the Left
        if (i - 1 >= 0 && i % 8 != 0)
            KingMap[i] |= 1ull << (i - 1);
        
        // Up
        if (i + 8 < SQUARE_NB)
            KingMap[i] |= 1ull << (i + 8);
        
        // Down
        if (i - 8 >= 0)
            KingMap[i] |= 1ull << (i - 8);
    }
}

void generateOccupancyMaskRook(){
    
    int i, bit;
    uint64_t mask;
    
    for (bit = 0, mask = 0; bit < SQUARE_NB; bit++, mask = 0){
        
        // Moving Upwards
        for (i = bit + 8; i <= 55; i += 8)
            mask |= (1ull << i);
        
        // Moving Downwards
        for (i = bit - 8; i >= 8; i -= 8)
            mask |= (1ull << i);
        
        // Moving to the Right
        for (i = bit + 1; i % 8 != 7 && i % 8 != 0; i++)
            mask |= (1ull << i);
        
        // Moving to the Left
        for (i = bit - 1; i % 8 != 7 && i % 8 !=0 && i >= 0; i--)
            mask |= (1ull << i);
        
        OccupancyMaskRook[bit] = mask;
    }
}

void generateOccupancyMaskBishop(){
    
    int i, bit; 
    uint64_t mask;
    
    for (bit = 0, mask = 0; bit < SQUARE_NB; bit++, mask = 0){
        
        // Moving Up and to the Right
        for (i = bit + 9; i % 8 != 7 && i % 8 != 0 && i <= 55; i += 9)
            mask |= (1ull << i);
        
        // Moving Down and to the Left
        for (i = bit - 9; i % 8 != 7 && i % 8 != 0 && i >=  8; i -= 9)
            mask |= (1ull << i);
        
        // Moving Up and to the Left
        for (i = bit + 7; i % 8 != 7 && i % 8 != 0 && i <= 55; i += 7)
            mask |= (1ull << i);
        
        // Moving Down and to the Right
        for (i = bit - 7; i % 8 != 7 && i % 8 !=0  && i >=  8; i -= 7)
            mask |= (1ull << i);
        
        OccupancyMaskBishop[bit] = mask;
    }
}

void generateOccupancyVariationsRook(){
    
    uint64_t mask;
    int i, j, sq, variationCount;
    int maskBits[20], indexBits[20];
    
    // Allocate space for the occupancy variations
    OccupancyVariationsRook = malloc(sizeof(uint64_t*) * SQUARE_NB);
    for (i = 0; i < SQUARE_NB; i++)
        OccupancyVariationsRook[i] = malloc(sizeof(uint64_t) * 4096);
    
    // Compute sets of variations for each square
    for (sq = 0; sq < SQUARE_NB; sq++){
        mask = OccupancyMaskRook[sq];
        getSetBits(mask, maskBits);
        variationCount = (int)(1ull << countSetBits(mask));
        
        // Compute for each possible variation on this square
        for (i = 0, mask = 0ull; i < variationCount; i++, mask = 0ull){
            getSetBits(i, indexBits);
            for (j = 0; indexBits[j] != -1; j++)
                mask |= (1ull << maskBits[indexBits[j]]);
            OccupancyVariationsRook[sq][i] = mask; 
        }
    }
}

void generateOccupancyVariationsBishop(){
    
    uint64_t mask;
    int i, j, sq, variationCount;
    int maskBits[20], indexBits[20];
    
    // Allocate space for the occupancy variations
    OccupancyVariationsBishop = malloc(sizeof(uint64_t*) * SQUARE_NB);
    for (i = 0; i < SQUARE_NB; i++)
        OccupancyVariationsBishop[i] = malloc(sizeof(uint64_t) * 512);
    
    // Compute sets of variations for each square
    for (sq = 0; sq < SQUARE_NB; sq++){
        mask = OccupancyMaskBishop[sq];
        getSetBits(mask, maskBits);
        variationCount = (int)(1ull << countSetBits(mask));
        
        // Compute for each possible variation on this square
        for (i = 0, mask = 0ull; i < variationCount; i++, mask = 0ull){
            getSetBits(i, indexBits);
            for (j = 0; indexBits[j] != -1; j++)
                mask |= 1ull << maskBits[indexBits[j]];
            OccupancyVariationsBishop[sq][i] = mask; 
        }
    }
}

void generateMoveDatabaseRook(){
    
    uint64_t moves, occupancy, magic;
    int i, j, sq, variations, tablesize, shift, index;
    
    // Allocate space for the rook's look up table
    tablesize = MagicRookIndexes[SQUARE_NB-1];
    tablesize += 1 << (64 - MagicShiftsRook[SQUARE_NB-1]);
    MoveDatabaseRook = malloc(sizeof(uint64_t) * tablesize);
    
    for (sq = 0; sq < SQUARE_NB; sq++){
        
        // Computer number of variations of blockers
        variations = (1 << countSetBits(OccupancyMaskRook[sq]));
        
        // Fetch the corresponding magic values for this square
        magic = MagicNumberRook[sq];
        shift = MagicShiftsRook[sq];
        
        // For each possible variation, compute the move list
        for (i = 0, moves = 0ull; i < variations; i++, moves = 0ull){
            
            // Fetch the blockers for this iteration
            occupancy = OccupancyVariationsRook[sq][i];
            
            // Compute the magic index
            index = (int)((occupancy * magic) >> shift);
            
            // Moving Upwards until we hit a blocker
            for (j = sq + 8; j < SQUARE_NB; j += 8) { 
                moves |= (1ull << j); 
                if (occupancy & (1ull << j))
                    break; 
            }
            
            // Moving Downwards until we hit a blocker
            for (j = sq - 8; j >= 0; j -= 8) { 
                moves |= (1ull << j); 
                if (occupancy & (1ull << j))
                    break; 
            }
            
            // Moving to the Right until we hit a blocker
            for (j = sq + 1; j % 8 != 0; j++) { 
                moves |= (1ull << j);
                if (occupancy & (1ull << j))
                    break;
            }
            
            // Moving to the Left until we hit a blocker
            for (j = sq - 1; j % 8 != 7 && j >= 0; j--) {
                moves |= (1ull << j);
                if (occupancy & (1ull << j))
                    break;
            }
            
            // Finally, save the moves for the square and blocker combination
            MoveDatabaseRook[MagicRookIndexes[sq] + index] = moves;
        }
    }
}

void generateMoveDatabaseBishop(){
    
    uint64_t moves, occupancy, magic;
    int i, j, sq, variations, tablesize, shift, index;
    
    // Allocate space for the bishop's look up table
    tablesize = MagicBishopIndexes[SQUARE_NB-1];
    tablesize += 1 << (64 - MagicShiftsBishop[SQUARE_NB-1]);
    MoveDatabaseBishop = malloc(sizeof(uint64_t) * tablesize);
    
    for (sq = 0; sq < SQUARE_NB; sq++){
        
        // Computer number of variations of blockers
        variations = (1 << countSetBits(OccupancyMaskBishop[sq]));
        
        // Fetch the corresponding magic values for this square
        magic = MagicNumberBishop[sq];
        shift = MagicShiftsBishop[sq];
        
        // For each possible variation, compute the move list
        for (i = 0, moves = 0ull; i < variations; i++, moves = 0ull){
            
            // Fetch the blockers for this iteration
            occupancy = OccupancyVariationsBishop[sq][i];
            
            // Compute the magic index
            index = (int)((occupancy * magic) >> shift);
            
            // Moving Upwards and to the Right until we hit a blocker
            for (j = sq + 9; j % 8 != 0 && j < SQUARE_NB; j += 9) { 
                moves |= (1ull << j); 
                if (occupancy & (1ull << j))
                    break; 
            }
            
            // Moving Downwards and to the Left until we hit a blocker
            for (j = sq - 9; j % 8 != 7 && j >= 0; j -= 9) { 
                moves |= (1ull << j); 
                if (occupancy & (1ull << j))
                    break; 
            }
            
            // Moving Upwards and to the Left until we hit a blocker
            for (j = sq + 7; j % 8 != 7 && j < SQUARE_NB; j += 7) { 
                moves |= (1ull << j); 
                if (occupancy & (1ull << j))
                    break; 
            }
            
            // Moving Downwards and to the Right until we hit a blocker
            for (j = sq - 7; j % 8 !=0 && j >= 0; j -= 7) { 
                moves |= (1ull << j); 
                if (occupancy & (1ull << j))
                    break; 
            }
            
            // Finally, save the moves for the square and blocker combination
            MoveDatabaseBishop[MagicBishopIndexes[sq] + index] = moves;
        }
    }
}
