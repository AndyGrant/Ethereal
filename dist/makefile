CCWIN32 = gcc32
CCWIN64 = gcc
CCDROID = arm-linux-gnueabihf-gcc


CWIN32FLAGS = -DNDEBUG -O3 -Wall -Wextra -pedantic -std=c99 -m32
CWIN64FLAGS = -DNDEBUG -O3 -Wall -Wextra -pedantic -std=c99 -m64
CDROIDFLAGS = -DNDEBUG -O3 -Wall -Wextra -pedantic -std=c99

LIBS = -static

SRC = ../src/bitboards.c       \
      ../src/bitutils.c        \
      ../src/board.c           \
      ../src/castle.c          \
      ../src/evaluate.c        \
      ../src/magics.c          \
      ../src/masks.c           \
      ../src/move.c            \
      ../src/movegen.c         \
      ../src/movegentest.c     \
      ../src/psqt.c            \
      ../src/search.c          \
      ../src/time.c            \
      ../src/transposition.c   \
      ../src/uci.c             \
      ../src/zorbist.c         \
      
all:
	$(CCDROID) $(CDROIDFLAGS) $(SRC) -o $(EXE)-Android $(LIBS)
	$(CCWIN32) $(CWIN32FLAGS) $(SRC) -o $(EXE)-Win32.exe $(LIBS)
	$(CCWIN64) $(CWIN64FLAGS) $(SRC) -o $(EXE)-Win64.exe $(LIBS)
