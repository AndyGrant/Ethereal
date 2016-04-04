#include <stdint.h>
#include "stdlib.h"

#include "magics.h"
#include "bitutils.h"

/*
 * 	Begings the process of initializing all magic bitboard
 *	components needed to create the end result move database
 *	lookup tables for move generation
 *
 *	Sets flag INITIALIZED_MAGICS (magics.h) to indicate
 *	that the lookup-tables have been initalized.
 */
void init_magics(){
	if (INITIALIZED_MAGICS)
		return;
	
	generate_knight_map();
	generate_king_map();
	generate_occupancy_mask_rook();
	generate_occupancy_mask_bishop();
	generate_occupancy_variations_rook();
	generate_occupancy_variations_bishop();
	generate_move_database_rook();
	generate_move_database_bishop();
	
	INITIALIZED_MAGICS = 1;
}

void generate_knight_map(){
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

void generate_king_map(){
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

void generate_occupancy_mask_rook(){
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

void generate_occupancy_mask_bishop(){	
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

void generate_occupancy_variations_rook(){
	int j, bitRef, variationCount;
	uint64_t mask, i;
	int setBitsInMask[20], setBitsInIndex[20];
	
	for (bitRef = 0; bitRef < 64; bitRef++){
		mask = OccupancyMaskRook[bitRef];
		get_set_bits(mask,setBitsInMask);
		variationCount = (int)(1ull << count_set_bits(mask));
		
		for (i = 0; i < variationCount; i++){
			OccupancyVariationsRook[bitRef][i] = 0; 
			get_set_bits(i,setBitsInIndex);
			for (j = 0; setBitsInIndex[j] != -1; j++){
				OccupancyVariationsRook[bitRef][i] |= (1ull << setBitsInMask[setBitsInIndex[j]]);
			}
		}
	}
}

void generate_occupancy_variations_bishop(){
	int j, bitRef, variationCount;
	uint64_t mask, i;
	int setBitsInMask[20], setBitsInIndex[20];
	
	for (bitRef = 0; bitRef < 64; bitRef++){
		mask = OccupancyMaskBishop[bitRef];
		get_set_bits(mask,setBitsInMask);
		variationCount = (int)(1ull << count_set_bits(mask));
		
		for (i = 0; i < variationCount; i++){
			OccupancyVariationsBishop[bitRef][i] = 0; 
			get_set_bits(i,setBitsInIndex);
			for (j = 0; setBitsInIndex[j] != -1; j++)
				OccupancyVariationsBishop[bitRef][i] |= (1ull << setBitsInMask[setBitsInIndex[j]]);
		}
	}
}

void generate_move_database_rook(){
	uint64_t validMoves;
	int variations, bitCount;
	int bitRef, i, j, magicIndex;
	
	MoveDatabaseRook = malloc(sizeof(uint64_t) * 64);
	
	for (bitRef=0; bitRef<=63; bitRef++){
		bitCount = count_set_bits(OccupancyMaskRook[bitRef]);
		variations = (int)(1ull << bitCount);
		
		MoveDatabaseRook[bitRef] = malloc(sizeof(uint64_t) * variations);

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

			MoveDatabaseRook[bitRef][magicIndex] = validMoves;
		}
	}
}


void generate_move_database_bishop(){
	uint64_t validMoves;
	int variations, bitCount;
	int bitRef, i, j, magicIndex;
	
	MoveDatabaseBishop = malloc(sizeof(uint64_t) * 64);
	
	for (bitRef=0; bitRef<=63; bitRef++){
		bitCount = count_set_bits(OccupancyMaskBishop[bitRef]);
		variations = (int)(1ull << bitCount);
		
		MoveDatabaseBishop[bitRef] = malloc(sizeof(uint64_t) * variations);
		
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

			MoveDatabaseBishop[bitRef][magicIndex] = validMoves;
		}
	}
}