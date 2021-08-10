# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#                                                                           #
#   Ethereal is a UCI chess playing engine authored by Andrew Grant.        #
#   <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>        #
#                                                                           #
#   Ethereal is free software: you can redistribute it and/or modify        #
#   it under the terms of the GNU General Public License as published by    #
#   the Free Software Foundation, either version 3 of the License, or       #
#   (at your option) any later version.                                     #
#                                                                           #
#   Ethereal is distributed in the hope that it will be useful,             #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of          #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           #
#   GNU General Public License for more details.                            #
#                                                                           #
#   You should have received a copy of the GNU General Public License       #
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.   #
#                                                                           #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

### =========================================================================
### Section 1. Flag Declarations. Detect NNUE/Classis and Release vs Native
### =========================================================================

CC   = gcc
SRC  = *.c pyrrhic/tbprobe.c
LIBS = -lpthread -lm
NN   = -DUSE_NNUE=0
EXE  = Ethereal

ifdef EVALFILE
    NN   = -DUSE_NNUE=1 -DEVALFILE=\"$(EVALFILE)\"
    SRC += nnue/*.c
endif

ifdef OWNER
    NN   = -DUSE_NNUE=1 -DLICENSE_OWNER=\"$(OWNER)\"
    SRC += nnue/*.c
endif

WFLAGS   = -std=gnu11 -Wall -Wextra -Wshadow
RFLAGS   = -O3 $(WFLAGS) -DNDEBUG -flto $(NN) -static
CFLAGS   = -O3 $(WFLAGS) -DNDEBUG -flto $(NN) -march=native
TFLAGS   = -O3 $(WFLAGS) -DNDEBUG -flto $(NN) -march=native -fopenmp -DTUNE
PGOFLAGS = -fno-asynchronous-unwind-tables

POPCNTFLAGS = -DUSE_POPCNT -mpopcnt
PEXTFLAGS   = -DUSE_PEXT -mbmi2 $(POPCNTFLAGS)

SSSE3FLAGS  = -DUSE_SSSE3 -msse -msse2 -msse3 -mssse3
AVXFLAGS    = -DUSE_AVX -mavx -msse4.1 $(SSSE3FLAGS)
AVX2FLAGS   = -DUSE_AVX2 -mavx2 -mfma $(AVXFLAGS)

### =========================================================================
### Section 2. Native Build Configuration [ Auto-Detection ]
### =========================================================================

PROPS = $(shell echo | $(CC) -march=native -E -dM -)

# Detect POPCNT and PEXT Instruction Support

ifneq ($(findstring __POPCNT__, $(PROPS)),)
	CFLAGS += -DUSE_POPCNT
endif

ifneq ($(findstring __BMI2__, $(PROPS)),)
	ifeq ($(findstring __znver1, $(PROPS)),)
		ifeq ($(findstring __znver2, $(PROPS)),)
			CFLAGS += -DUSE_PEXT
		endif
	endif
endif

# Detect AVX2, AVX, or otherwise SSSE3 Instruction Support

ifneq ($(findstring __AVX2__, $(PROPS)),)
	CFLAGS += -DUSE_AVX2
endif

ifneq ($(findstring __AVX__, $(PROPS)),)
	CFLAGS += -DUSE_AVX
endif

ifneq ($(findstring __SSSE3__, $(PROPS)),)
	CFLAGS += -DUSE_SSSE3
endif

### =========================================================================
### Section 3. Build Targets Optimized For Native Use
### =========================================================================

default:
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $(EXE)

pgo:
	rm -f *.gcda pyrrhic/*.gcda nnue/*.gcda
	$(CC) -fprofile-generate $(CFLAGS) $(PGOFLAGS) $(SRC) $(LIBS) -o $(EXE)
	./$(EXE) bench
	$(CC) -fprofile-use $(CFLAGS) $(PGOFLAGS) $(SRC) $(LIBS) -o $(EXE)
	rm -f *.gcda pyrrhic/*.gcda nnue/*.gcda

tune:
	$(CC) $(TFLAGS) $(SRC) $(LIBS) -o $(EXE)

### =========================================================================
### Section 4. Release Build Targets [ make release OWNER= OS= EXE= EXT= ]
### =========================================================================

builddir:
	mkdir -p ../$(OWNER)/$(OS)

ssse3-popcnt: builddir
	$(CC) $(RFLAGS) $(SRC) $(LIBS) $(POPCNTFLAGS) $(SSSE3FLAGS) -o $(EXE)-ssse3$(EXT)

avx-popcnt: builddir
	$(CC) $(RFLAGS) $(SRC) $(LIBS) $(PEXTFLAGS)	  $(SSSE3FLAGS) -o $(EXE)-pext-ssse3$(EXT)

avx2-popcnt: builddir
	$(CC) $(RFLAGS) $(SRC) $(LIBS) $(POPCNTFLAGS) $(AVXFLAGS)	-o $(EXE)-avx$(EXT)

ssse3-pext: builddir
	$(CC) $(RFLAGS) $(SRC) $(LIBS) $(PEXTFLAGS)	  $(AVXFLAGS)	-o $(EXE)-pext-avx$(EXT)

avx-pext: builddir
	$(CC) $(RFLAGS) $(SRC) $(LIBS) $(POPCNTFLAGS) $(AVX2FLAGS)	-o $(EXE)-avx2$(EXT)

avx2-pext: builddir
	$(CC) $(RFLAGS) $(SRC) $(LIBS) $(PEXTFLAGS)	  $(AVX2FLAGS)	-o $(EXE)-pext-avx2$(EXT)

release: ssse3-popcnt avx-popcnt avx2-popcnt ssse3-pext avx-pext avx2-pext
