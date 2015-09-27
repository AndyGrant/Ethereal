from Tkinter import *
from subprocess import PIPE, Popen
import time, sys

ENGINE_A = sys.argv[1]+"/getEngineMove.exe";
ENGINE_B = sys.argv[2]+"/getEngineMove.exe";

ENGINE_DIRS = [ENGINE_A, ENGINE_B]

EXIT_ON_COMPLETION = bool(sys.argv[3])

WHITE = 0
BLACK = 1

STARTING_BOARD = "3111214151211131010101010101010199999999999999999999999999999999999999999999999999999999999999990000000000000000301020405020103000111199 1"

# StaleMate Test
# STARTING_BOARD =  "9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999950999999999999999999995100000099 0"

root = Tk()

IMAGES = {}
IMAGES['00'] = PhotoImage(file="../GUI/_Assets/WhitePawn.gif")
IMAGES['10'] = PhotoImage(file="../GUI/_Assets/WhiteKnight.gif")
IMAGES['20'] = PhotoImage(file="../GUI/_Assets/WhiteBishop.gif")
IMAGES['30'] = PhotoImage(file="../GUI/_Assets/WhiteRook.gif")
IMAGES['40'] = PhotoImage(file="../GUI/_Assets/WhiteQueen.gif")
IMAGES['50'] = PhotoImage(file="../GUI/_Assets/WhiteKing.gif")
IMAGES['01'] = PhotoImage(file="../GUI/_Assets/BlackPawn.gif")
IMAGES['11'] = PhotoImage(file="../GUI/_Assets/BlackKnight.gif")
IMAGES['21'] = PhotoImage(file="../GUI/_Assets/BlackBishop.gif")
IMAGES['31'] = PhotoImage(file="../GUI/_Assets/BlackRook.gif")
IMAGES['41'] = PhotoImage(file="../GUI/_Assets/BlackQueen.gif")
IMAGES['51'] = PhotoImage(file="../GUI/_Assets/BlackKing.gif")
IMAGES['OPTION'] = PhotoImage(file="../GUI/_Assets/Option.gif")
IMAGES["BOARD"] = PhotoImage(file="../GUI/_Assets/Board.gif")

class ChessGUI(Frame):
  def __init__(self, parent):
    self.parent = parent
    Frame.__init__(self,parent)
    self.initInterface(parent)
    self.game = ChessGame(STARTING_BOARD,int (not int(STARTING_BOARD[-1])),self.canvas)
    
    
  def initInterface(self, parent):
    self.parent.title("Ethereal Chess : Andrew Grant")
    self.pack(fill=BOTH,expand=1)
    self.canvas = Canvas(self,width=420,height=420)
    self.canvas.pack()
    
    
class ChessGame():
  def __init__(self, board, turn, canvas):
    self.canvas = canvas;
    self.selected = None
    self.board = board
    self.turn = turn
    self.drawn = []
    self.pastBoards = []
    
    
    self.drawBoard()
    Tk.update(root)
    
    while True:
      status = self.makeEngineMove()
      
      if self.stalemate():
        self.logGame(.5)
        break
      elif status == -1:
        self.logGame(0 if turn == WHITE else 1)
        break
      elif status == -2:
        self.logGame(.5)
        break
      
      self.drawBoard()
      Tk.update(root)
      self.turn = (1+self.turn)%2   

  def stalemate(self):
    for board in self.pastBoards:
      if self.pastBoards.count(board) >= 4:
        return True
    return False
      
  
  def logGame(self, result):
    with open('output.txt','a') as file:
      file.write(" ".join([sys.argv[1],"VS",sys.argv[2],"Result=",str(result),"\n"]))
    if EXIT_ON_COMPLETION:
      sys.exit(1)
      
  def drawBoard(self):
    for f in self.drawn:
      self.canvas.delete(f)
      
    self.drawn.append(self.canvas.create_image((0,0),anchor=NW,image=IMAGES["BOARD"]))
    
    if self.selected != None:
      for f in self.moves.getEnds(self.selected):
        self.drawn.append(self.canvas.create_image((10+50*(f%8),10+50*(f/8)),anchor=NW,image=IMAGES['OPTION']))
    
    for f in range(0,128,2):
      p = self.board[f] + self.board[f+1]
      if p != "99":
        x,y = 10 + 50 * (f/2/8), 10 + 50 * (f/2%8)
        self.drawn.append(self.canvas.create_image((y,x),anchor=NW,image=IMAGES[p]))    
    
  def makeEngineMove(self):
    proc = Popen([ENGINE_DIRS[self.turn],self.board,str((self.turn))], stdout=PIPE,shell=False)
    (out, err) = proc.communicate()
    #print out[0:out.find("NEWBOARD=")]
    if "CHECKMATE" in out:
      return -1
    if "STALEMATE" in out:
      return -2
    self.board = out[out.find("NEWBOARD=") + len("NEWBOARD="):]
    print self.board + " " + str(self.turn)
    self.pastBoards.append(self.board+" "+str(self.turn))
    return 1
    
root.geometry("420x420")
root.resizable(0,0)
app = ChessGUI(root)
root.mainloop()
