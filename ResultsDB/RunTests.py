import sys
import os

from ChessDataBase import ChessDataBase

DATABASE = ChessDataBase("DataBase.json")

CUTE_CHESS = "cutechess-cli.exe "
EXTRA_ARGS = "-each restart=on dir=../Engines tc=40/10 -games {0} -concurrency {1} -recover -wait 200 "
BOOK = "-openings file=book.pgn order=random plies=16"

ENGINE_DIR = "../Engines"

ENGINES = [
    ["BikJump2.01.exe", "uci", 2100],
    ["Clarabit1.0.exe", "uci", 2100],
    [   "Glass1.3.exe", "uci", 2358],
    ["MadChess2.0.exe", "uci", 2288],
    [ "Monarch1.7.exe", "uci", 2056],
    ["Sungorus1.4.exe", "uci", 2308],
]

TEST_ENGINE = [sys.argv[1], "uci"]
TEST_CMD = "-engine cmd={0} proto={1} ".format(*TEST_ENGINE)

GAMES = (sys.argv[2])
CORES = (sys.argv[3])

EXTRA_ARGS = EXTRA_ARGS.replace("{0}", GAMES)
EXTRA_ARGS = EXTRA_ARGS.replace("{1}", CORES)

index = -1
testNum = -1;
while (True):
    index = (index + 1) % len(ENGINES)
    testNum += 1
    engine = ENGINES[index]
    their_cmd = "-engine cmd={0} proto={1} ".format(*(engine[0:2]))
    cmd = cmd = CUTE_CHESS + TEST_CMD + their_cmd + EXTRA_ARGS + BOOK
    os.system(cmd + " > Logs/myTemp{0}.txt".format(testNum))
    
    with open("Logs/myTemp{0}.txt".format(testNum)) as fin:
        line = fin.readlines()
        line = line[-3].split(":")[1]
        line = line.replace(" ","")
        line = line.replace("[","-")
        w, l, d = map(int, line.split("-")[0:3])
    
    DATABASE.addGames(sys.argv[1], engine[0], engine[2], w, l, d)
    DATABASE.saveDataBase()