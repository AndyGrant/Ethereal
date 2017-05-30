#!/bin/sh

# Ethereal is a UCI chess playing engine authored by Andrew Grant.
# <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>
# 
# Ethereal is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Ethereal is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

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

echo "CC = gcc"                                               > $1/Source/makefile
echo "CFLAGS = -DNDEBUG -O3 -Wall -Wextra -Wshadow -std=c99" >> $1/Source/makefile
echo "SRC = *.c"                                             >> $1/Source/makefile
echo "all:"                                                  >> $1/Source/makefile
echo "	\$(CC) \$(CFLAGS) \$(SRC) -o $1"                     >> $1/Source/makefile

cd ../ 