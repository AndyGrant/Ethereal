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

#include <stdbool.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <sys/time.h>
#endif

#include "types.h"

struct TimeManager {
    int pv_stability;
    double start_time, ideal_usage, max_usage;
    uint64_t nodes[0x10000];
};

double get_real_time();
double elapsed_time(const TimeManager *tm);
void tm_init(const Limits *limits, TimeManager *tm);
void tm_update(const Thread *thread, const Limits *limits, TimeManager *tm);
bool tm_finished(const Thread *thread, const TimeManager *tm);
bool tm_stop_early(const Thread *thread);
