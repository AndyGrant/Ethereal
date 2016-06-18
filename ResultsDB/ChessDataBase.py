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
        
        outputStr = ""
        OPP_FORMAT_STR = "{:>25} {:>5}; {:>5}; {:>5}; {:>5}; {:>5}; {:>7};\n"
        
        engines = [f for f in self.data]
        engines.sort()
        
        for engine in engines:
            
            opponents = self.data[engine]
            
            ordered = [f for f in opponents]
            ordered.sort()
            
            tempStr = ""
            
            names  = []; wins  = []
            losses = []; draws = []
            elos   = []; diffs = []
            finals = []
            
            for opponent in ordered:
                names  += [opponent]
                wins   += [int(opponents[opponent]["Wins"])]
                losses += [int(opponents[opponent]["Losses"])]
                draws  += [int(opponents[opponent]["Draws"])]
                elos   += [int(opponents[opponent]["ELO"])]
                diffs  += [self.calculateELODifference(wins[-1], losses[-1], draws[-1])]
                finals += [elos[-1] + diffs[-1]]
                
            
            tempStr += OPP_FORMAT_STR.format("","Win","Loss","Draw","Elo","Diff","Final")
                
            tempStr += OPP_FORMAT_STR.format(engine,"","","","","",sum(finals)/len(finals))
                
            for f in range(len(names)):
                n, w, l, d, e, di, f = zip(names,wins,losses,draws,elos,diffs,finals)[f]
                tempStr += OPP_FORMAT_STR.format(n, w, l, d, e, di, f)
                
            outputStr += tempStr + "\n\n"
            
        with open(filename, "w") as fout:
            fout.write(outputStr)
            
    def calculateELODifference(self, *args):
        wins, losses, draws = map(float, args)
        score = (wins + (draws * .5)) / (wins + losses + draws)
        return int(round((-400 * math.log10((1/score) - 1))))
        
if __name__ == "__main__":
    ChessDataBase("DataBase.json").outputDataBase("EloTables.txt")
