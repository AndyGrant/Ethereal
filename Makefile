all:
	gcc -g -p -pg  -DNDEBUG -Wall -Wno-unused-variable -Wno-format src/uci.cpp src/board.c src/move.c src/util.c src/search.c -o Ethereal -lstdc++

