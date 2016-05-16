CC = gcc

CFLAGS = -Wall -Wno-unused-variable -Wno-format -m64 -march=native -O3 -DNDEBUG

PFLAGS = -p -g -pg

profile = False

SRC = bitutils.c		\
	  board.c			\
	  evaluate.c		\
	  magics.c			\
	  main.c			\
	  masks.c			\
	  move.c			\
	  movegen.c			\
	  movegentest.c		\
	  psqt.c			\
	  search.c			\
	  transposition.c	\
	  zorbist.c			\
	  
EXE = foo

ifeq (true, $(profile))
	FLAGS = $(CFLAGS) $(PFLAGS)
else
	FLAGS = $(CFLAGS)
endif

all:
	$(CC) $(FLAGS) $(SRC) -o $(EXE)
