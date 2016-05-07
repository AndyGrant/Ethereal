CC = gcc

CFLAGS = -m64 -march=native -O3 -DNDEBUG

WFLAGS = -Wall -Wno-unused-variable -Wno-format

PFLAGS = -p -g -pg

profile = False

SRC = bitutils.c		\
	  board.c			\
	  evaluate.c		\
	  magics.c			\
	  main.c			\
	  move.c			\
	  movegen.c			\
	  movegentest.c		\
	  psqt.c			\
	  search.c			\
	  transposition.c	\
	  zorbist.c			\
	  
EXE = foo

ifeq (true, $(profile))
	FLAGS = $(WFLAGS) $(CFLAGS) $(PFLAGS)
else
	FLAGS = $(WFLAGS) $(CFLAGS)
endif

all:
	$(CC) $(FLAGS) $(SRC) -o $(EXE)
