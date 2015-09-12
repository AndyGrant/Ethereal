from Tkinter import *
from subprocess import PIPE, Popen

GETMOVES_DIR = "..\\Engine\\getMoves.exe"


WHITE = 0
BLACK = 1
STARTING_BOARD = "31112141512111310101010101010101999999999999999999999999999999999999999999999999999999999999999900000000000000003010204050201030001111"

root = Tk()

IMAGES = {}
IMAGES['00'] = PhotoImage(file="_Assets//WhitePawn.gif")
IMAGES['10'] = PhotoImage(file="_Assets//WhiteKnight.gif")
IMAGES['20'] = PhotoImage(file="_Assets//WhiteBishop.gif")
IMAGES['30'] = PhotoImage(file="_Assets//WhiteRook.gif")
IMAGES['40'] = PhotoImage(file="_Assets//WhiteQueen.gif")
IMAGES['50'] = PhotoImage(file="_Assets//WhiteKing.gif")
IMAGES['01'] = PhotoImage(file="_Assets//BlackPawn.gif")
IMAGES['11'] = PhotoImage(file="_Assets//BlackKnight.gif")
IMAGES['21'] = PhotoImage(file="_Assets//BlackBishop.gif")
IMAGES['31'] = PhotoImage(file="_Assets//BlackRook.gif")
IMAGES['41'] = PhotoImage(file="_Assets//BlackQueen.gif")
IMAGES['51'] = PhotoImage(file="_Assets//BlackKing.gif")
IMAGES["BOARD"] = PhotoImage(file="_Assets//Board.gif")

class ChessGUI(Frame):
	def __init__(self, parent):
		self.parent = parent
		Frame.__init__(self,parent)
		self.initInterface(parent)
		self.game = ChessGame(STARTING_BOARD,WHITE,self.canvas)
		
	def initInterface(self, parent):
		self.parent.title("Ethereal Chess : Andrew Grant")
		self.pack(fill=BOTH,expand=1)
		self.canvas = Canvas(self,width=420,height=420)
		self.canvas.pack()
		
		
class ChessGame():
	def __init__(self, board, turn, canvas):
		self.canvas = canvas;
		self.canvas.bind("<Button-1>", self.onClick)
		self.board = board
		self.turn = turn
		self.drawn = []
		self.moves = self.getMoves()
		self.drawBoard()
		
	def drawBoard(self):
		for f in self.drawn:
			self.canvas.delete(f)
			
		self.drawn.append(self.canvas.create_image((0,0),anchor=NW,image=IMAGES["BOARD"]))
		for f in range(0,128,2):
			p = self.board[f] + self.board[f+1]
			if p != "99":
				z = f/2;
				x = 10 + 50 * (z/8)
				y = 10 + 50 * (z%8)
				self.drawn.append(self.canvas.create_image((y,x),anchor=NW,image=IMAGES[p]))
				
	def onClick(self,event):
		x = self.getClicked(event)
		if x is -1: return
		
		
	def getClicked(self,event):
		x,y = event.x-10, event.y-10
		if x < 0 or y < 0 or x > 400 or y > 400:
			return -1
		return (x/50)+(8*(y/50))
		
	def getMoves(self):
		proc = Popen([GETMOVES_DIR,self.board,str(self.turn)], stdout=PIPE,shell=True)
		(out, err) = proc.communicate()
		data = out.split("\r\n")
		moves = [f.split(" ") for f in data]
		for f in moves: print f
	
root.geometry("420x420")
root.resizable(0,0)
app = ChessGUI(root)
root.mainloop()