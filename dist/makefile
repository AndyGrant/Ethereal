CCWIN32 = gcc32
CCWIN64 = gcc
CCDROID = arm-linux-gnueabi-gcc


CWIN32FLAGS = -DNDEBUG -O3 -Wall -Wextra -pedantic -std=c99 -m32
CWIN64FLAGS = -DNDEBUG -O3 -Wall -Wextra -pedantic -std=c99 -m64
CDROIDFLAGS = -DNDEBUG -O3 -Wall -Wextra -pedantic -std=c99 -march=armv5t
LIBS = -static

SRC = ../src/*.c
      
all:
	$(CCDROID) $(CDROIDFLAGS) $(SRC) -o $(EXE)-Android $(LIBS)
	$(CCWIN32) $(CWIN32FLAGS) $(SRC) -o $(EXE)-Win32.exe $(LIBS)
	$(CCWIN64) $(CWIN64FLAGS) $(SRC) -o $(EXE)-Win64.exe $(LIBS)
