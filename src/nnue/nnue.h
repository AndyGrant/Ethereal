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

#include "../types.h"

#if USE_NNUE

void nnue_init(const char* fname);
void nnue_incbin_init();
int nnue_evaluate(Thread *thread, Board *board);

#else

INLINE void nnue_init(const char* fname) {
    (void) fname; printf("info string Error: NNUE is disabled for this binary\n");
}

INLINE void nnue_incbin_init() {
    (void) 0;
};

INLINE int nnue_evaluate(Thread *thread, Board * board) {
    (void) thread; (void) board; return 0;
}

#endif
