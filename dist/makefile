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

CCWIN32 = gcc32
CCWIN64 = gcc
CCDROID = arm-linux-gnueabi-gcc

CWIN32FLAGS = -DNDEBUG -O3 -Wall -Wextra -Wshadow -std=c99 -m32
CWIN64FLAGS = -DNDEBUG -O3 -Wall -Wextra -Wshadow -std=c99 -m64
CDROIDFLAGS = -DNDEBUG -O3 -Wall -Wextra -Wshadow -std=c99 -march=armv5t

LIBS = -static
SRC = ../src/*.c
      
all:
	$(CCDROID) $(CDROIDFLAGS) $(SRC) -o $(EXE)-Android $(LIBS)
	$(CCWIN32) $(CWIN32FLAGS) $(SRC) -o $(EXE)-Win32.exe $(LIBS)
	$(CCWIN64) $(CWIN64FLAGS) $(SRC) -o $(EXE)-Win64.exe $(LIBS)
