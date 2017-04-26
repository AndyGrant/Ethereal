#!/bin/sh

mkdir $1
mkdir $1/Win32
mkdir $1/Win64
mkdir $1/Android
mkdir $1/Source

make EXE=$1

mv $1-Win32.exe $1/Win32/$1.exe
mv $1-Win64.exe $1/Win64/$1.exe
mv $1-Android $1/Android/$1

cp -r ../src/* $1/Source

echo "CC = gcc"                                                > $1/Source/makefile
echo "CFLAGS = -DNDEBUG -O3 -Wall -Wextra -pedantic -std=c99" >> $1/Source/makefile
echo "SRC = *.c"                                              >> $1/Source/makefile
echo "all:"                                                   >> $1/Source/makefile
echo "	\$(CC) \$(CFLAGS) \$(SRC) -o $1"                      >> $1/Source/makefile

cd ../ 