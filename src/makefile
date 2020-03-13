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

CC      = gcc
SRC     = *.c fathom/tbprobe.c
LIBS    = -lpthread -lm
EXE     = Ethereal

WFLAGS = -std=gnu11 -Wall -Wextra -Wshadow
CFLAGS = -DNDEBUG -O3 $(WFLAGS) -march=native -flto
RFLAGS = -DNDEBUG -O3 $(WFLAGS) -flto -static
TFLAGS = -DNDEBUG -O3 $(WFLAGS) -march=native -flto -fopenmp -DTUNE
PFLAGS = -DNDEBUG -O0 $(WFLAGS) -p -pg
DFLAGS = -O0 $(WFLAGS)

POPCNTFLAGS = -DUSE_POPCNT -msse3 -mpopcnt
PGO_FLAGS = -DNDEBUG -O3 $(WFLAGS) -march=native -flto -static -fprofile-generate -lgcov
PGO_BFLAGS = -DNDEBUG -O3 $(WFLAGS) -march=native -flto -static -fprofile-use -fno-peel-loops -fno-tracer -lgcov
PEXTFLAGS   = $(POPCNTFLAGS) -DUSE_PEXT -mbmi2

popcnt:
	$(CC) $(CFLAGS) $(SRC) $(LIBS) $(POPCNTFLAGS) -o $(EXE)

nopopcnt:
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $(EXE)

pext:
	$(CC) $(CFLAGS) $(SRC) $(LIBS) $(PEXTFLAGS) -o $(EXE)

release:
	mkdir ../dist
	$(CC) $(RFLAGS) $(SRC) $(LIBS) -o ../dist/$(EXE)$(VER)-x64-nopopcnt.exe
	$(CC) $(RFLAGS) $(SRC) $(LIBS) $(POPCNTFLAGS) -o ../dist/$(EXE)$(VER)-x64-popcnt.exe
	$(CC) $(RFLAGS) $(SRC) $(LIBS) $(PEXTFLAGS) -o ../dist/$(EXE)$(VER)-x64-pext.exe

texel:
	$(CC) $(TFLAGS) $(SRC) $(LIBS) $(POPCNT) -o $(EXE)

profile:
	$(CC) $(PFLAGS) $(SRC) $(LIBS) $(POPCNT) -o $(EXE)

profile-gen:
	$(CC)  $(SRC) $(LIBS) $(POPCNT) $(PGO_FLAGS) -o $(EXE)

profile-use:
	$(CC)  $(SRC) $(LIBS) $(POPCNT) $(PGO_BFLAGS) -o $(EXE)

bench:
	$(EXE) bench >/dev/null

clean:
	rm *gcda
	strip $(EXE)*


pgo:
	make profile-gen
	make bench
	make profile-use
	make bench
	make clean
	@echo ""
	@echo "Success - PGO Build completed! "

debug:
	$(CC) $(DFLAGS) $(SRC) $(LIBS) $(POPCNT) -o $(EXE)
