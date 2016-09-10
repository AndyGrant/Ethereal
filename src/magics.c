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

#include <stdint.h>
#include <stdlib.h>

#include "magics.h"
#include "bitutils.h"

uint64_t KnightMap[64];
uint64_t KingMap[64];

uint64_t OccupancyMaskRook[64];
uint64_t OccupancyMaskBishop[64];

uint64_t ** OccupancyVariationsRook;
uint64_t ** OccupancyVariationsBishop;

uint64_t * MoveDatabaseRook;
uint64_t * MoveDatabaseBishop;

int MagicRookIndexes[64] = {
        0,  4096,  6144,  8192, 10240, 12288, 14336, 16384,
    20480, 22528, 23552, 24576, 25600, 26624, 27648, 28672,
    30720, 32768, 33792, 34816, 35840, 36864, 37888, 38912,
    40960, 43008, 44032, 45056, 46080, 47104, 48128, 49152,
    51200, 53248, 54272, 55296, 56320, 57344, 58368, 59392,
    61440, 63488, 64512, 65536, 66560, 67584, 68608, 69632,
    71680, 73728, 74752, 75776, 76800, 77824, 78848, 79872,
    81920, 86016, 88064, 90112, 92160, 94208, 96256, 98304
};

int MagicBishopIndexes[64] = {
       0,   64,   96,  128,  160,  192,  224,  256,
     320,  352,  384,  416,  448,  480,  512,  544,
     576,  608,  640,  768,  896, 1024, 1152, 1184,
    1216, 1248, 1280, 1408, 1920, 2432, 2560, 2592,
    2624, 2656, 2688, 2816, 3328, 3840, 3968, 4000,
    4032, 4064, 4096, 4224, 4352, 4480, 4608, 4640,
    4672, 4704, 4736, 4768, 4800, 4832, 4864, 4896,
    4928, 4992, 5024, 5056, 5088, 5120, 5152, 5184
};

int MagicShiftsRook[64] = {
    52,53,53,53,53,53,53,52,53,54,54,54,54,54,54,53,
    53,54,54,54,54,54,54,53,53,54,54,54,54,54,54,53,
    53,54,54,54,54,54,54,53,53,54,54,54,54,54,54,53,
    53,54,54,54,54,54,54,53,52,53,53,53,53,53,53,52
};

int MagicShiftsBishop[64] = {
    58,59,59,59,59,59,59,58,59,59,59,59,59,59,59,59,
    59,59,57,57,57,57,59,59,59,59,57,55,55,57,59,59,
    59,59,57,55,55,57,59,59,59,59,57,57,57,57,59,59,
    59,59,59,59,59,59,59,59,58,59,59,59,59,59,59,58
};

uint64_t MagicNumberRook[64] = {
    0xa180022080400230, 0x0040100040022000, 0x0080088020001002, 0x0080080280841000,
    0x4200042010460008, 0x04800a0003040080, 0x0400110082041008, 0x008000a041000880,
    0x10138001a080c010, 0x0000804008200480, 0x00010011012000c0, 0x0022004128102200,
    0x000200081201200c, 0x202a001048460004, 0x0081000100420004, 0x4000800380004500,
    0x0000208002904001, 0x0090004040026008, 0x0208808010002001, 0x2002020020704940,
    0x8048010008110005, 0x6820808004002200, 0x0a80040008023011, 0x00b1460000811044,
    0x4204400080008ea0, 0xb002400180200184, 0x2020200080100380, 0x0010080080100080,
    0x2204080080800400, 0x0000a40080360080, 0x02040604002810b1, 0x008c218600004104,
    0x8180004000402000, 0x488c402000401001, 0x4018a00080801004, 0x1230002105001008,
    0x8904800800800400, 0x0042000c42003810, 0x008408110400b012, 0x0018086182000401,
    0x2240088020c28000, 0x001001201040c004, 0x0a02008010420020, 0x0010003009010060,
    0x0004008008008014, 0x0080020004008080, 0x0282020001008080, 0x50000181204a0004,
    0x0102042111804200, 0x40002010004001c0, 0x0019220045508200, 0x200030010060a900,
    0x0008018028040080, 0x0088240002008080, 0x0010301802830400, 0x00332a4081140200,
    0x008080010a601241, 0x0001008010400021, 0x0004082001007241, 0x0211009001200509,
    0x8015001002441801, 0x0801000804000603, 0x0c0900220024a401, 0x0001000200608243
};

uint64_t MagicNumberBishop[64] = {
    0x2910054208004104, 0x02100630a7020180, 0x5822022042000000, 0x2ca804a100200020,
    0x0204042200000900, 0x2002121024000002, 0x80404104202000e8, 0x812a020205010840,
    0x8005181184080048, 0x1001c20208010101, 0x1001080204002100, 0x1810080489021800,
    0x0062040420010a00, 0x5028043004300020, 0xc0080a4402605002, 0x08a00a0104220200,
    0x0940000410821212, 0x001808024a280210, 0x040c0422080a0598, 0x4228020082004050,
    0x0200800400e00100, 0x020b001230021040, 0x00090a0201900c00, 0x004940120a0a0108,
    0x0020208050a42180, 0x001004804b280200, 0x2048020024040010, 0x0102c04004010200,
    0x020408204c002010, 0x02411100020080c1, 0x102a008084042100, 0x0941030000a09846,
    0x0244100800400200, 0x4000901010080696, 0x0000280404180020, 0x0800042008240100,
    0x0220008400088020, 0x04020182000904c9, 0x0023010400020600, 0x0041040020110302,
    0x0412101004020818, 0x8022080a09404208, 0x1401210240484800, 0x0022244208010080,
    0x1105040104000210, 0x2040088800c40081, 0x8184810252000400, 0x4004610041002200,
    0x040201a444400810, 0x4611010802020008, 0x80000b0401040402, 0x0020004821880a00,
    0x8200002022440100, 0x0009431801010068, 0x1040c20806108040, 0x0804901403022a40,
    0x2400202602104000, 0x0208520209440204, 0x040c000022013020, 0x2000104000420600,
    0x0400000260142410, 0x0800633408100500, 0x00002404080a1410, 0x0138200122002900
};

/**
 * Initalize the Knight and King attack lookup tables.
 * Initalize the needed data members for the Rook and
 * Bishop attack lookup tables. Initalize the Rook and
 * Bishop attack lookup tables. Set a flag so the process
 * Is not repeated more than once.
 */
void initalizeMagics(){
    
    int i;
    uint64_t j;
    
    generateKnightMap();
    generateKingMap();
    generateOccupancyMaskRook();
    generateOccupancyMaskBishop();
    generateOccupancyVariationsRook();
    generateOccupancyVariationsBishop();
    generateMoveDatabaseRook();
    generateMoveDatabaseBishop();
    
    // CLEAN UP OCCUPANCY VARIATIONS FOR BISHOPS
    for (i = 0; i < 64; i++)
        free(OccupancyVariationsBishop[i]);
    free(OccupancyVariationsBishop);
    
    // CLEAN UP OCCUPANCY VARIATIONS FOR ROOKS
    for (i = 0; i < 64; i++)
        free(OccupancyVariationsRook[i]);
    free(OccupancyVariationsRook);
    
    
    for (j = 0ull; j < 0x10000ull; j++)
        BitCounts[j] = countSetBits(j);
}

/**
 * Fill the KnightMap[64] array with the correct 
 * BitBoards for generating knight moves
 */
void generateKnightMap(){
    
    int i;
    uint64_t z = 1;
    
    for(i = 0; i < 64; i++){
        if (i + 17 < 64 && i % 8 != 7)
            KnightMap[i] |= z << ( i + 17);
        if (i - 17 >= 0 && i % 8 != 0)
            KnightMap[i] |= z << ( i - 17);
        if (i + 15 < 64 && i % 8 != 0)
            KnightMap[i] |= z << ( i + 15);
        if (i - 15 >= 0 && i % 8 != 7)
            KnightMap[i] |= z << ( i - 15);
        if (i + 10 < 64 && i % 8 <= 5)
            KnightMap[i] |= z << ( i + 10);
        if (i - 10 >= 0 && i % 8 >= 2)
            KnightMap[i] |= z << ( i - 10);
        if (i + 6  < 64 && i % 8 >= 2)
            KnightMap[i] |= z << ( i + 6);
        if (i - 6  >= 0 && i % 8 <= 5)
            KnightMap[i] |= z << ( i - 6);
    }
}

/**
 * Fill the KingMap[64] array with the correct 
 * BitBoards for generating king moves
 */
void generateKingMap(){
    
    int i;
    uint64_t z = 1;
    
    for(i = 0; i < 64; i++){
        if (i + 9 < 64 && i % 8 != 7)
            KingMap[i] |= z << (i + 9);
        if (i - 9 >= 0 && i % 8 != 0)
            KingMap[i] |= z << (i - 9); 
        if (i + 7 < 64 && i % 8 != 0)
            KingMap[i] |= z << (i + 7);
        if (i - 7 >= 0 && i % 8 != 7)
            KingMap[i] |= z << (i - 7); 
        if (i + 1 < 64 && i % 8 != 7)
            KingMap[i] |= z << (i + 1);
        if (i - 1 >= 0 && i % 8 != 0)
            KingMap[i] |= z << (i - 1);
        if (i + 8 < 64)
            KingMap[i] |= z << (i + 8);
        if (i - 8 >= 0)
            KingMap[i] |= z << (i - 8);
    }
}

/**
 * Fill the OccupancyMaskRook[64] array with the correct
 * BitBoards for what what be the proper moves if the only
 * piece on the board was the rook in question. This is needed
 * to create the occupancy variations for rooks.
 */
void generateOccupancyMaskRook(){
    
    int i, bit;
    uint64_t mask;
    
    for (bit = 0, mask = 0; bit <= 63; bit++, mask = 0){
        for (i=bit+8; i<=55; i+=8)
            mask |= (1ull << i);
        for (i=bit-8; i>=8; i-=8)
            mask |= (1ull << i);
        for (i=bit+1; i%8!=7 && i%8!=0 ; i++)
            mask |= (1ull << i);
        for (i=bit-1; i%8!=7 && i%8!=0 && i>=0; i--)
            mask |= (1ull << i);
        OccupancyMaskRook[bit] = mask;
    }
}

/**
 * Fill the OccupancyMaskBishop[64] array with the correct
 * BitBoards for what what be the proper moves if the only
 * piece on the board was the bishop in question. This is needed
 * to create the occupancy variations for bishops.
 */
void generateOccupancyMaskBishop(){
    
    int i, bit; 
    uint64_t mask;
    
    for (bit = 0, mask = 0; bit <= 63; bit++, mask = 0){
        for (i=bit+9; i%8!=7 && i%8!=0 && i<=55; i+=9)
            mask |= (1ull << i);
        for (i=bit-9; i%8!=7 && i%8!=0 && i>=8; i-=9)
            mask |= (1ull << i);
        for (i=bit+7; i%8!=7 && i%8!=0 && i<=55; i+=7)
            mask |= (1ull << i);
        for (i=bit-7; i%8!=7 && i%8!=0 && i>=8; i-=7)
            mask |= (1ull << i);
        OccupancyMaskBishop[bit] = mask;
    }
}

/**
 * Fill the OccupancyVariationsRook array with each possible
 * Set of attacks based on potential blockers along the rook's
 * sliding path.
 */
void generateOccupancyVariationsRook(){
    
    int i, j, bitRef, variationCount;
    uint64_t mask;
    int setBitsInMask[20], setBitsInIndex[20];
    
    OccupancyVariationsRook = malloc(sizeof(uint64_t) * 64);
    
    for (i = 0; i < 64; i++)
        OccupancyVariationsRook[i] = malloc(sizeof(uint64_t) * 4096);
    
    for (bitRef = 0; bitRef < 64; bitRef++){
        mask = OccupancyMaskRook[bitRef];
        getSetBits(mask,setBitsInMask);
        variationCount = (int)(1ull << countSetBits(mask));
        
        for (i = 0; i < variationCount; i++){
            OccupancyVariationsRook[bitRef][i] = 0; 
            getSetBits(i,setBitsInIndex);
            for (j = 0; setBitsInIndex[j] != -1; j++){
                OccupancyVariationsRook[bitRef][i] |= (1ull << setBitsInMask[setBitsInIndex[j]]);
            }
        }
    }
}

/**
 * Fill the OccupancyVariationsBishop array with each possible
 * Set of attacks based on potential blockers along the bishop's
 * sliding path.
 */
void generateOccupancyVariationsBishop(){
    
    int i, j, bitRef, variationCount;
    uint64_t mask;
    int setBitsInMask[20], setBitsInIndex[20];
    
    OccupancyVariationsBishop = malloc(sizeof(uint64_t) * 64);
    
    for (i = 0; i < 64; i++)
        OccupancyVariationsBishop[i] = malloc(sizeof(uint64_t) * 512);
    
    for (bitRef = 0; bitRef < 64; bitRef++){
        mask = OccupancyMaskBishop[bitRef];
        getSetBits(mask,setBitsInMask);
        variationCount = (int)(1ull << countSetBits(mask));
        
        for (i = 0; i < variationCount; i++){
            OccupancyVariationsBishop[bitRef][i] = 0; 
            getSetBits(i,setBitsInIndex);
            for (j = 0; setBitsInIndex[j] != -1; j++)
                OccupancyVariationsBishop[bitRef][i] |= (1ull << setBitsInMask[setBitsInIndex[j]]);
        }
    }
}

/**
 * Fill the MoveDatabaseRook tables so that the moves
 * for a given rook can be found by calculating the 
 * database index and current location, then accessing
 * the table at MoveDatabaseRook[location][index], and 
 * then finally bit-wise anding it with empty | enemy
 */
void generateMoveDatabaseRook(){
    
    uint64_t validMoves;
    int variations, bitCount;
    int bitRef, i, j, magicIndex;
    
    MoveDatabaseRook = malloc(sizeof(uint64_t) * (98304 + 4096));
    
    for (bitRef=0; bitRef<=63; bitRef++){
        bitCount = countSetBits(OccupancyMaskRook[bitRef]);
        variations = (int)(1ull << bitCount);
        
        for (i = 0; i < variations; i++){
            validMoves = 0;
            magicIndex = (int)((OccupancyVariationsRook[bitRef][i] * MagicNumberRook[bitRef]) >> MagicShiftsRook[bitRef]);
            
            for (j=bitRef+8; j<=63; j+=8) { 
                validMoves |= (1ull << j); 
                if ((OccupancyVariationsRook[bitRef][i] & (1ull << j)) != 0) 
                    break; 
            }
            
            for (j=bitRef-8; j>=0; j-=8) { 
                validMoves |= (1ull << j); 
                if ((OccupancyVariationsRook[bitRef][i] & (1ull << j)) != 0) 
                    break; 
            }
            
            for (j=bitRef+1; j%8!=0; j++) { 
                validMoves |= (1ull << j);
                if ((OccupancyVariationsRook[bitRef][i] & (1ull << j)) != 0) 
                    break;
            }
                
            for (j=bitRef-1; j%8!=7 && j>=0; j--) {
                validMoves |= (1ull << j);
                if ((OccupancyVariationsRook[bitRef][i] & (1ull << j)) != 0)
                    break;
            }
            
            MoveDatabaseRook[MagicRookIndexes[bitRef] + magicIndex] = validMoves;
        }
    }
}

/**
 * Fill the MoveDatabaseBishop tables so that the moves
 * for a given bishop can be found by calculating the 
 * database index and current location, then accessing
 * the table at MoveDatabaseBishop[location][index], and 
 * then finally bit-wise anding it with empty | enemy
 */
void generateMoveDatabaseBishop(){
    
    uint64_t validMoves;
    int variations, bitCount;
    int bitRef, i, j, magicIndex;
    
    MoveDatabaseBishop = malloc(sizeof(uint64_t) * (5184 + 64));
    
    for (bitRef=0; bitRef<=63; bitRef++){
        bitCount = countSetBits(OccupancyMaskBishop[bitRef]);
        variations = (int)(1ull << bitCount);
        
        for (i = 0; i < variations; i++){
            validMoves = 0;
            magicIndex = (int)((OccupancyVariationsBishop[bitRef][i] * MagicNumberBishop[bitRef]) >> MagicShiftsBishop[bitRef]);
            
            for (j=bitRef+9; j%8!=0 && j<=63; j+=9) { 
                validMoves |= (1ull << j); 
                if ((OccupancyVariationsBishop[bitRef][i] & (1ull << j)) != 0)
                    break; 
            }
            
            for (j=bitRef-9; j%8!=7 && j>=0; j-=9) { 
                validMoves |= (1ull << j); 
                if ((OccupancyVariationsBishop[bitRef][i] & (1ull << j)) != 0) 
                    break; 
            }
            
            for (j=bitRef+7; j%8!=7 && j<=63; j+=7) { 
                validMoves |= (1ull << j); 
                if ((OccupancyVariationsBishop[bitRef][i] & (1ull << j)) != 0) 
                    break; 
            }
            
            for (j=bitRef-7; j%8!=0 && j>=0; j-=7) { 
                validMoves |= (1ull << j); 
                if ((OccupancyVariationsBishop[bitRef][i] & (1ull << j)) != 0) 
                    break; 
            }
            
            MoveDatabaseBishop[MagicBishopIndexes[bitRef] + magicIndex] = validMoves;
        }
    }
}