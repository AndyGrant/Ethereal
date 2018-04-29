# Ethereal is a UCI chess playing engine authored by Andrew Grant.
# <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>
# 
# Ethereal is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Ethereal is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

CC = gcc

CCDROID = arm-linux-gnueabi-gcc

SRC = *.c fathom/tbprobe.c 

LIBS = -lpthread -lm

EXE = Ethereal


WFLAGS = -std=c11 -Wall -Wextra -Wshadow

CFLAGS = -DNDEBUG -O3 $(WFLAGS) -march=native -flto

RFLAGS = -DNDEBUG -O3 $(WFLAGS) -flto -static

PFLAGS = -DNDEBUG -O0 $(WFLAGS) -p -pg

DFLAGS = -O0 $(WFLAGS) -std=c99

TFLAGS = $(CFLAGS) -fopenmp -DTUNE

POPCNT = -msse3 -mpopcnt -DUSE_POPCOUNT


all:
	$(CC) $(CFLAGS) $(SRC) $(LIBS) $(POPCNT) -o $(EXE)
    
profile:
	$(CC) $(PFLAGS) $(SRC) $(LIBS) $(POPCNT) -o $(EXE)
    
debug:
	$(CC) $(DFLAGS) $(SRC) $(LIBS) $(POPCNT) -o $(EXE)
    
tune:
	$(CC) $(TFLAGS) $(SRC) $(LIBS) $(POPCNT) -o $(EXE)
    
release:
	$(CC) $(RFLAGS) $(SRC) $(LIBS) -o $(EXE)$(VER)-Win64-NoPop.exe
	$(CC) $(RFLAGS) $(SRC) $(LIBS) $(POPCNT) -o $(EXE)$(VER)-Win64.exe
	$(CCDROID) $(RFLAGS) $(SRC) -march=armv5t $(LIBS) -o $(EXE)$(VER)-Android.exe
