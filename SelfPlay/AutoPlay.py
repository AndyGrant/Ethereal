import os, sys

gn = 0
while True:
  os.system("python SelfPlay.py " + sys.argv[1] + " " + sys.argv[2] + " True > AGame" + str(gn))
  gn += 1
  os.system("python SelfPlay.py " + sys.argv[2] + " " + sys.argv[1] + " True > AGame" + str(gn))
  gn += 1