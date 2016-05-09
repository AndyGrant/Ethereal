CC = gcc

CFLAGS = 

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
	FLAGS = $(CFLAGS) $(PFLAGS)
else
	FLAGS = $(CFLAGS)
endif

all:
	$(CC) $(FLAGS) $(SRC) -o $(EXE)
