#!/bin/sh

gcc -o3 Engine.c GetMoves.c -o GetMoves.exe
gcc -o3 Search.c TTable.c Engine.c getEngineMove.c -o getEngineMove.exe -lpthreadgc2

OS="`uname`"
case $OS in
  'Linux')
   #gcc -o3 Engine.c TTable.c Search.c GetBestMove.c -o GetBestMove.exe -lpthread;;
   ;;
  *) 
	#gcc -o3 Engine.c TTable.c Search.c GetBestMove.c -o GetBestMove.exe -lpthreadGC2;;
	;;
esac


#3111214151999931999999999999999999999921991199990101010101010101000000000000000099999920991099999999999999999999301020405099993000111199
