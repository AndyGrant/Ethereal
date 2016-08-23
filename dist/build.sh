#!/bin/sh

mkdir $1
cd $1

mkdir Win32
cd Win32
mkdir Source
cd ../

mkdir Win64
cd Win64
mkdir Source
cd ../

mkdir Android
cd Android
mkdir Source
cd ../

mkdir Linux
cd Linux
mkdir Source
cd ../

cd ../
make EXE=$1
cd $1

cp ../$1-Win32.exe Win32
cp ../$1-Win64.exe Win64
cp ../$1-Android Android

rm ../$1-Win32.exe
rm ../$1-Win64.exe
rm ../$1-Android

cp -r ../../src/* Win32/Source/
cp -r ../../src/* Win64/Source/
cp -r ../../src/* Android/Source/
cp -r ../../src/* Linux/Source/

echo "CC = gcc"                                               >> Linux/Source/makefile
echo "CFLAGS = -DNDEBUG -O3 -Wall -Wextra -pedantic -std=c99" >> Linux/Source/makefile
echo "SRC = bitboards.c       \\"                             >> Linux/Source/makefile
echo "      bitutils.c        \\"                             >> Linux/Source/makefile
echo "      board.c           \\"                             >> Linux/Source/makefile
echo "      castle.c          \\"                             >> Linux/Source/makefile
echo "      evaluate.c        \\"                             >> Linux/Source/makefile
echo "      magics.c          \\"                             >> Linux/Source/makefile
echo "      masks.c           \\"                             >> Linux/Source/makefile
echo "      move.c            \\"                             >> Linux/Source/makefile
echo "      movegen.c         \\"                             >> Linux/Source/makefile
echo "      movegentest.c     \\"                             >> Linux/Source/makefile
echo "      psqt.c            \\"                             >> Linux/Source/makefile
echo "      search.c          \\"                             >> Linux/Source/makefile
echo "      time.c            \\"                             >> Linux/Source/makefile
echo "      transposition.c   \\"                             >> Linux/Source/makefile
echo "      uci.c             \\"                             >> Linux/Source/makefile
echo "      zorbist.c         \\"                             >> Linux/Source/makefile
echo ""                                                       >> Linux/Source/makefile
echo "all:"                                                   >> Linux/Source/makefile
echo "	\$(CC) \$(CFLAGS) \$(SRC) -o ../$1-Linux"             >> Linux/Source/makefile

cd ../ 