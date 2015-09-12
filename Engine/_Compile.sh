#!/bin/sh

gcc -o3 Engine.c GetMoves.c -o GetMoves.exe
gcc -o3 Search.c TTable.c Engine.c getEngineMove.c -o getEngineMove.exe -lpthreadgc2

OS="`uname`"
case $OS in
  'Linux')
    gcc -o3 Engine.c GetMoves.c -o GetMoves.exe
    gcc -o3 Search.c TTable.c Engine.c getEngineMove.c -o getEngineMove.exe -lpthread
  ;;
  
  *) 
    gcc -o3 Engine.c GetMoves.c -o GetMoves.exe
    gcc -o3 Search.c TTable.c Engine.c getEngineMove.c -o getEngineMove.exe -lpthreadgc2
  ;;
esac