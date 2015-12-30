all:
	gcc -p -p -pg  src/uci.cpp src/board.c src/move.c src/util.c src/search.c -o Ethereal -lstdc++

