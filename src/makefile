CC = gcc
CFLAGS = -DNDEBUG -O3 -Wall -Wextra -pedantic -std=c99

PFLAGS = -DNDEBUG -Wall -Wextra -pedantic -std=c99 -p -pg

SRC = bitboards.c       \
      bitutils.c        \
      board.c           \
      castle.c          \
      evaluate.c        \
      magics.c          \
      masks.c           \
      move.c            \
      movegen.c         \
      movegentest.c     \
      psqt.c            \
      search.c          \
      time.c            \
      transposition.c   \
      uci.c             \
      zorbist.c         \


all:
	$(CC) $(CFLAGS) $(SRC) -o Ethereal.exe
    
profile:
	$(CC) $(PFLAGS) $(SRC) -o Ethereal.exe
