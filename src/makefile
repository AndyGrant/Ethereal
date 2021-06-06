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

CC   = gcc
SRC  = *.c pyrrhic/tbprobe.c
LIBS = -lpthread -lm
EXE  = Ethereal

ifdef EVALFILE
    NN = -DUSE_NNUE=1 -DEVALFILE=\"$(EVALFILE)\"
    SRC += nnue/*.c
else
    NN = -DUSE_NNUE=0
endif

WFLAGS   = -std=gnu11 -Wall -Wextra -Wshadow
RFLAGS   = -O3 $(WFLAGS) $(NN) -DNDEBUG -flto -static
CFLAGS   = -O3 $(WFLAGS) $(NN) -DNDEBUG -flto -march=native
TFLAGS   = -O3 $(WFLAGS) $(NN) -DNDEBUG -flto -march=native -fopenmp -DTUNE
PGOFLAGS = -O3 $(WFLAGS) $(NN) -DNDEBUG -flto -march=native -fno-asynchronous-unwind-tables

POPCNTFLAGS = -DUSE_POPCNT -msse3 -mpopcnt
PEXTFLAGS   = $(POPCNTFLAGS) -DUSE_PEXT -mbmi2

SSSE3FLAGS  = -DUSE_SSSE3 -msse -msse2 -mssse3
AVXFLAGS    = -DUSE_AVX $(SSSE3FLAGS) -msse4.1 -mavx
AVX2FLAGS   = -DUSE_AVX2 $(AVXFLAGS) -mavx2 -mfma

ARMV8FLAGS  = -O3 $(WFLAGS) -DNDEBUG -flto -march=armv8-a -m64
ARMV7FLAGS  = -O3 $(WFLAGS) -DNDEBUG -flto -march=armv7-a -m32
ARMV7FLAGS += -mfloat-abi=softfp -mfpu=vfpv3-d16 -mthumb -Wl,--fix-cortex-a8

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

PROPS = $(shell echo | $(CC) -march=native -E -dM -)

ifneq ($(findstring __AVX2__, $(PROPS)),)
	CFLAGS += -DUSE_AVX2
endif

ifneq ($(findstring __AVX__, $(PROPS)),)
	CFLAGS += -DUSE_AVX
endif

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

popcnt:
	$(CC) $(CFLAGS) $(SRC) $(LIBS) $(POPCNTFLAGS) -o $(EXE)

pext:
	$(CC) $(CFLAGS) $(SRC) $(LIBS) $(PEXTFLAGS) -o $(EXE)

pgo:
	rm -f *.gcda pyrrhic/*.gcda nnue/*.gcda
	$(CC) -fprofile-generate $(PGOFLAGS) $(SRC) $(LIBS) $(POPCNTFLAGS) -o $(EXE)
	./$(EXE) bench
	$(CC) -fprofile-use $(PGOFLAGS) $(SRC) $(LIBS) $(POPCNTFLAGS) -o $(EXE)
	rm -f *.gcda pyrrhic/*.gcda nnue/*.gcda

pgopext:
	rm -f *.gcda pyrrhic/*.gcda nnue/*.gcda
	$(CC) -fprofile-generate $(PGOFLAGS) $(SRC) $(LIBS) $(PEXTFLAGS) -o $(EXE)
	./$(EXE) bench
	$(CC) -fprofile-use $(PGOFLAGS) $(SRC) $(LIBS) $(PEXTFLAGS) -o $(EXE)
	rm -f *.gcda pyrrhic/*.gcda nnue/*.gcda

armv8:
	$(CC) $(ARMV8FLAGS) $(SRC) -lm -o $(EXE)

armv7:
	$(CC) $(ARMV7FLAGS) $(SRC) -lm -o $(EXE)

release:
	$(CC) $(RFLAGS) $(SRC) $(LIBS) $(POPCNTFLAGS) $(AVXFLAGS) -o $(EXE)$(VER)-popcnt-avx
	$(CC) $(RFLAGS) $(SRC) $(LIBS) $(PEXTFLAGS) $(AVXFLAGS) -o $(EXE)$(VER)-pext-avx
	$(CC) $(RFLAGS) $(SRC) $(LIBS) $(POPCNTFLAGS) $(AVX2FLAGS) -o $(EXE)$(VER)-popcnt-avx2
	$(CC) $(RFLAGS) $(SRC) $(LIBS) $(PEXTFLAGS) $(AVX2FLAGS) -o $(EXE)$(VER)-pext-avx2

tune:
	$(CC) $(TFLAGS) $(SRC) $(LIBS) $(POPCNT) -o $(EXE)

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #