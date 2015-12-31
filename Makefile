all:
	gcc -O3 -DNDEBUG src/uci.cpp src/board.c src/move.c src/util.c src/search.c src/evaluate.c -o Ethereal -lstdc++

