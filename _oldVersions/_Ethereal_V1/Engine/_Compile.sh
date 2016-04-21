#!/bin/sh

OS="`uname`"
case $OS in
  'Linux')
    gcc -o3 Board.c Move.c MoveGen.c GetMoves.c -o getMoves.exe
    gcc -o3 Board.c Move.c MoveGen.c TTable.c Evaluate.c Search.c GetEngineMove.c -o getEngineMove.exe -lpthread
  ;;
  
  *) 
    gcc -o3 Board.c Move.c MoveGen.c GetMoves.c -o getMoves.exe
    gcc -o3 Board.c Move.c MoveGen.c TTable.c Evaluate.c Search.c GetEngineMove.c -o getEngineMove.exe -lpthreadgc2
  ;;
esac
