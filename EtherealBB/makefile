CC = gcc

CFLAGS = -m64 -march=native -DNDEBUG -O3

SRC = bitutils.c        \
      board.c           \
      evaluate.c        \
      magics.c          \
      main.c            \
      move.c            \
      movegen.c         \
      movegentest.c     \
      psqt.c            \
      search.c          \
      transposition.c   \
      zorbist.c         \
      
EXE = foo

all:
	$(CC) $(CFLAGS) $(SRC) -o $(EXE)
	
