from Tkinter import *
from subprocess import PIPE, Popen
import time

GETMOVES_DIR = "..\\Engine\\getMoves.exe"
GETENGINEMOVE_DIR = "..\\Engine\\getEngineMove.exe"


WHITE = 0
BLACK = 1
STARTING_BOARD = "3111214151211131010101010101010199999999999999999999999999999999999999999999999999999999999999990000000000000000301020405020103000111199"

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
IMAGES['OPTION'] = PhotoImage(file="_Assets//Option.gif")
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
		self.selected = None
		self.board = board
		self.turn = turn
		self.drawn = []
		self.moves = Moves(self.board,self.turn)
		print self.board
		self.drawBoard()
		
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
	
	def onClick(self,event):
		x = self.getClicked(event)
		if x is -1: return
		if self.selected == None:
			self.selected = x
			self.drawBoard()
		elif x in self.moves.getEnds(self.selected):
			self.board = self.moves.getBoard(self.selected,x)
			self.selected = None
			self.drawBoard()
			Tk.update(root)
			self.makeEngineMove()
			self.drawBoard()
			print self.board
		else:
			self.selected = None
			self.drawBoard()
			
		
	def getClicked(self,event):
		x,y = event.x-10, event.y-10
		if x < 0 or y < 0 or x > 400 or y > 400:
			return -1
		return (x/50)+(8*(y/50))
		
	def makeEngineMove(self):
		
		proc = Popen([GETENGINEMOVE_DIR,self.board,str((self.turn+1)%2)], stdout=PIPE,shell=True)
		(out, err) = proc.communicate()
		print out
		self.board = out[out.find("NEWBOARD=") + len("NEWBOARD="):]
		self.moves = Moves(self.board,self.turn)
		
		
class Moves():
	def __init__(self, board, turn):
		proc = Popen([GETMOVES_DIR,board,str(turn)], stdout=PIPE,shell=True)
		(out, err) = proc.communicate()
		data = out.split("\r\n")
		data = [f.split(" ") for f in data]
		self.moves = [Move(f) for f in data]
		
	def __str__(self):
		return "\n".join([f.__str__() for f in self.moves])
		
	def hasStart(self, start):
		for f in self.moves:
			if f[0] == str(start):
				return True
		return False
		
	def getEnds(self, start):
		return [int(f[1]) for f in self.moves if f[0] == str(start)]
		
	def getBoard(self, start, end):
		for f in self.moves:
			if str(start) == f[0]:
				if str(end) == f[1]:
					return f[4]
		print "Error"
class Move():
	def __init__(self, data):
		self.data = data[0:5]
		
	def __str__(self):
		return " ".join(self.data)
	
	def __getitem__(self, index):
		return self.data[index]
	
root.geometry("420x420")
root.resizable(0,0)
app = ChessGUI(root)
root.mainloop()