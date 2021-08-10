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

#define vepi8  __m128i
#define vepi16 __m128i
#define vepi32 __m128i
#define vps32  __m256

#define vepi8_cnt  16
#define vepi16_cnt 8
#define vepi32_cnt 4
#define vps32_cnt  8

#define vepi16_add   _mm_add_epi16
#define vepi16_sub   _mm_sub_epi16
#define vepi16_max   _mm_max_epi16
#define vepi16_madd  _mm_madd_epi16
#define vepi16_one   _mm_set1_epi16(1)
#define vepi16_zero  _mm_setzero_si128
#define vepi16_srai  _mm_srai_epi16
#define vepi16_packu _mm_packus_epi16
#define vepi16_maubs _mm_maddubs_epi16

#define vepi32_add  _mm_add_epi32
#define vepi32_max  _mm_max_epi32
#define vepi32_hadd _mm_hadd_epi32
#define vepi32_zero _mm_setzero_si128

#define vps32_add  _mm256_add_ps
#define vps32_mul  _mm256_mul_ps
#define vps32_max  _mm256_max_ps
#define vps32_hadd _mm256_hadd_ps
#define vps32_zero _mm256_setzero_ps

#define vps32_fma(A, B, C) vps32_add(vps32_mul(A, B), C)
