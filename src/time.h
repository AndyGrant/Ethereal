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

#ifndef _MY_TIME_H
#define _MY_TIME_H

#include "types.h"

double getRealTime();
double elapsedTime(SearchInfo* info);
void initTimeManagment(SearchInfo* info, Limits* limits);
void updateTimeManagment(SearchInfo* info, Limits* limits, int depth, int value);
int terminateTimeManagment(SearchInfo* info);

static const double PVFactorCount  = 8;
static const double PVFactorWeight = 0.085;

#endif
