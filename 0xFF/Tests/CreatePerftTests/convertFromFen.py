import sys

def convertFromFen(fen):

	def getPieces(s):
		board = ""
		for f in s:
			if f != "/":
				if f not in "12345678":
					board += f
				else:
					board += "e" * int(f)
		return board

	def getTurn(s):
		return ['1','0'][s=='w' or s=='W']
		
	def getRights(s):
		wk = int("K" in s)
		wq = int("Q" in s)
		bk = int("k" in s)
		bq = int("q" in s)
		return str(wk)+str(wq)+str(bk)+str(bq)
		
	def getEnpass(s):
		if s == "-":
			return '000'
		let = int(s[0] - 'a')
		num = [4,5][s[1] == '3']
		val = (16*(4+num)) + 4 + let
		
		return str(val/100)+str((val%100)/10)+str((val%10)/1)

	chunks = fen.split(" ")
	pieces = getPieces(chunks[0])
	turn = getTurn(chunks[1])
	rights = getRights(chunks[2])
	ep = getEnpass(chunks[3])

	board = pieces+rights+ep+turn

	return board
