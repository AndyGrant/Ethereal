all:
	gcc -O3 -march=native -DNDEBUG src/globals.c src/uci.cpp src/board.c src/move.c src/util.c src/search.c src/evaluate.c src/zorbist.c -o Ethereal -lstdc++

