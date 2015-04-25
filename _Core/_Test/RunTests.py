import os

CASE_SPLIT_CHAR = "= \n"
LINE_SPLIT_CHAR = "&"

class TestSuite:
	def __init__(self, file, executable, testType):
		self.file = file
		self.executable = executable
		self.testType = testType
		
	def executeTests(self):
		input = open(self.file,'r')
		data = input.readlines()
		input.close()
		
		casesEvaluated = 0
		for case in data[0:len(data)-1]:
			self.testType(case).executeTest(self.executable)
			casesEvaluated += 1
			
			if casesEvaluated % 50 == 0:
				print "\r" + str(casesEvaluated) + " of " + str(len(data)-1) + " ",
		
		print "\r" + str(casesEvaluated) + " of " + str(casesEvaluated) + " "
		print "Finished All Tests"
			

class EngineTest:
	def __init__(self, data):
		self.board = self.createBoard(data)
		self.output = self.createOutput(data)		
		
	def createBoard(self, data):
		return data.split(" ")[0]
		
	def createOutput(self, data):
		return " ".join(data.split(" ")[1:-1])
		
	def executeTest(self, executable):
		input = executable + " " + self.board + " " + self.output
		os.system(input)
		
		
suits = [
	["./_TestData/EngineTests.txt","./EngineTests.exe",EngineTest],
]

for suit in suits:
	TestSuite(suit[0],suit[1],suit[2]).executeTests()
