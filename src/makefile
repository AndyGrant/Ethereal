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

EXE = Ethereal

WFLAGS = -Wall -Wextra -Wshadow

CFLAGS = -DNDEBUG -O3 $(WFLAGS) -std=c99 -march=native -flto

RFLAGS = -DNDEBUG -O3 $(WFLAGS) -std=c99 -flto -static

PFLAGS = -DNDEBUG -O0 $(WFLAGS) -std=c99 -p -pg

DFLAGS = -O0 $(WFLAGS) -std=c99

TFLAGS = $(CFLAGS) -fopenmp -DTUNE

SRC = *.c

all:
	$(CC) $(CFLAGS) $(SRC) -lpthread -o $(EXE)
    
profile:
	$(CC) $(PFLAGS) $(SRC) -lpthread -o $(EXE)
    
debug:
	$(CC) $(DFLAGS) $(SRC) -lpthread -o $(EXE)
    
tune:
	$(CC) $(TFLAGS) $(SRC) -lpthread -lm -o $(EXE)
    
release:
	$(CC) $(RFLAGS) $(SRC) -lpthread -o $(EXE)$(VER)-Win64.exe
    
android:
	$(CCDROID) $(RFLAGS) $(SRC) -march=armv5t -lpthread -o $(EXE)$(VER)-Android.exe
