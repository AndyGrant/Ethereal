#!/bin/sh

if [[ "$unamestr" == 'Linux' ]]; then
	gcc -o3 TTable.c Search.c Engine.c -o foo.exe -L. -lpthread
else
	gcc -o3 TTable.c Search.c Engine.c -o foo.exe -L. -lpthreadGC2
fi
