/******************************************************************************/
/*                                                                            */
/*    Ethereal is a UCI chess playing engine authored by Andrew Grant.        */
/*    <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>        */
/*                                                                            */
/*    Ethereal is free software: you can redistribute it and/or modify        */
/*    it under the terms of the GNU General Public License as published by    */
/*    the Free Software Foundation, either version 3 of the License, or       */
/*    (at your option) any later version.                                     */
/*                                                                            */
/*    Ethereal is distributed in the hope that it will be useful,             */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/*    GNU General Public License for more details.                            */
/*                                                                            */
/*    You should have received a copy of the GNU General Public License       */
/*    along with this program.  If not, see <http://www.gnu.org/licenses/>    */
/*                                                                            */
/******************************************************************************/

#pragma once

#include <stdlib.h>

#include "../types.h"

#ifdef USE_AVX2

    #define vepi16      __m256i
    #define vepi16_add  _mm256_add_epi16
    #define vepi16_sub  _mm256_sub_epi16
    #define vepi16_max  _mm256_max_epi16
    #define vepi16_madd _mm256_madd_epi16
    #define vepi16_zero _mm256_setzero_si256
    #define vepi16_cnt  16

    #define vepi32      __m256i
    #define vepi32_add  _mm256_add_epi32
    #define vepi32_max  _mm256_max_epi32
    #define vepi32_hadd _mm256_hadd_epi32
    #define vepi32_zero _mm256_setzero_si256
    #define vepi32_cnt  8

    #define vps32_fma(A, B, C) _mm256_fmadd_ps(A, B, C)

#elif USE_AVX

    #define vepi16      __m128i
    #define vepi16_add  _mm_add_epi16
    #define vepi16_sub  _mm_sub_epi16
    #define vepi16_max  _mm_max_epi16
    #define vepi16_madd _mm_madd_epi16
    #define vepi16_zero _mm_setzero_si128
    #define vepi16_cnt  8

    #define vepi32      __m128i
    #define vepi32_add  _mm_add_epi32
    #define vepi32_max  _mm_max_epi32
    #define vepi32_hadd _mm_hadd_epi32
    #define vepi32_zero _mm_setzero_si128
    #define vepi32_cnt  4

    #define vps32_fma(A, B, C) _mm256_add_ps(_mm256_mul_ps(A, B), C)

#endif

#define INSIZE  40960
#define KPSIZE  256
#define L1SIZE  512
#define L2SIZE  32
#define L3SIZE  32
#define OUTSIZE 1

typedef struct NNUEDelta {
    int piece, from, to;
} NNUEDelta;

typedef struct NNUEAccumulator {
    int accurate, changes;
    NNUEDelta deltas[3];
    ALIGN64 int16_t values[2][KPSIZE];
} NNUEAccumulator;
