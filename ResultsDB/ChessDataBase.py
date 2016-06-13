import json
import sys
import math

class ChessDataBase():

    def __init__(self, filename):
        self.data = None
        self.filename = None
        
        self.loadDataBase(filename)
        
    def loadDataBase(self, filename):
        try:
            with open(filename) as fin:
                rawData = fin.readlines()
                parsed = "".join(rawData)
                self.data = json.loads(parsed)
                
                self.filename = filename
                print "-->Loaded DataBase :", filename
        except:
            print "-->Unable to load DataBase :", filename
            sys.exit()
        
    def hasEngine(self, engineName):
        assert self.data != None
        return engineName in self.data
        
    def hasOpponent(self, engineName, opponentName):
        assert self.hasEngine(engineName)
        return opponentName in self.data[engineName]
        
    def addEngine(self, engineName):
        assert not self.hasEngine(engineName)
        self.data[engineName] = {}
        print "-->Added Engine :", engineName
    
    def addOpponent(self, engineName, opponentName, opponentELO):
        assert not self.hasOpponent(engineName, opponentName)
        self.data[engineName][opponentName] = {
            "ELO"    : str(opponentELO),
            "Wins"   : "0",
            "Losses" : "0",
            "Draws"  : "0"
        }
        print "-->Added Opponent :", opponentName, "For :", engineName
        
    def addGames(self, engineName, opponentName, opponentELO, wins, losses, draws):
        if not self.hasEngine(engineName):
            self.addEngine(engineName)
            
        if not self.hasOpponent(engineName, opponentName):
            self.addOpponent(engineName, opponentName, opponentELO)
            
        seg = self.data[engineName][opponentName]
        
        seg["Wins"]   = str(int(seg["Wins"])   + wins)
        seg["Losses"] = str(int(seg["Losses"]) + losses)
        seg["Draws"]  = str(int(seg["Draws"])  + draws)
        
        print "-->Added Games :", engineName, "V", opponentName, ":", wins, losses, draws
    
    def saveDataBase(self):
        
        with open(self.filename) as fin:
            rawData = fin.readlines()
            parsed = "".join(rawData)
            data = json.loads(parsed)
            
            with open("temp"+self.filename, "w") as fout:
                json.dump(data , fout)
    
        with open(self.filename, "w") as fout:
            json.dump(self.data, fout)
            
    def outputDataBase(self, filename):
    
        FORMAT_STR = "{:30}; {:>3}; {:>3}; {:>3}; {:>5}; {:>5}; {:>7};"
        
        outputStr = ""
        
        for engine in self.data:
            opponents = self.data[engine]
            
            matchups = []
            
            for opponent in opponents:
                name = opponent
                wins = opponents[opponent]["Wins"]
                losses = opponents[opponent]["Losses"]
                draws = opponents[opponent]["Draws"]
                elo = int(opponents[opponent]["ELO"])
                diff = self.calculateELODifference(wins, losses, draws)
                final = elo + diff
                matchups += [FORMAT_STR.format(name,wins,losses,draws,elo,diff,final)]
                
            outputStr += FORMAT_STR.format(engine,"W","L","D","ELO","DIFF","FINAL") + "\n"
            outputStr += "\n".join(matchups)
            outputStr += "\n\n"
            
        with open(filename, "w") as fout:
            fout.write(outputStr)
            
    def calculateELODifference(self, *args):
        wins, losses, draws = map(float, args)
        score = (wins + (draws * .5)) / (wins + losses + draws)
        return int(round((-400 * math.log10((1/score) - 1))))
        
if __name__ == "__main__":
    ChessDataBase("DataBase.json").outputDataBase("Foo.txt")