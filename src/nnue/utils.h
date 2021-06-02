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

#if defined(_WIN32) || defined(_WIN64)

INLINE void* align_malloc(size_t size) {
    return _mm_malloc(size, 64);
}

INLINE void align_free(void *ptr) {
    _mm_free(ptr);
}

#else

INLINE void* align_malloc(size_t size) {
    void *mem; return posix_memalign(&mem, 64, size) ? NULL : mem;
}

INLINE void align_free(void *ptr) {
    free(ptr);
}

#endif
