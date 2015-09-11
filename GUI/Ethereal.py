from Tkinter import *

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
		Frame.__init__(self, parent, background="white")   
		self.parent = parent
		self.initUI(parent)
		self.boardStr = "31112141512111310101010101010101999999999999999999999999999999999999999999999999999999999999999900000000000000003010204050201030001111"
		self.drawBoard(self.boardStr)

	def initUI(self, parent):
		self.parent.title("Ethereal Chess : Andrew Grant")
		self.pack(fill=BOTH, expand=1)
		self.canvas = Canvas(self,width=420,height=420)
		self.canvas.pack()
		
	def drawBoard(self,str):
		self.canvas.create_image((0,0),anchor=NW,image=IMAGES["BOARD"])
		for f in range(0,128,2):
			p = str[f] + str[f+1]
			if p != "99":
				z = f/2;
				x = 10 + 50 * (z/8)
				y = 10 + 50 * (z%8)
				self.canvas.create_image((y,x),anchor=NW,image=IMAGES[p])

	
root.geometry("420x420")
root.resizable(0,0)
app = ChessGUI(root)
root.mainloop()  

