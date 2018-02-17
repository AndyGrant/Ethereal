import time
import subprocess

process = subprocess.Popen(
    "Ethereal.exe",
    stderr=subprocess.STDOUT,
    stdout=subprocess.PIPE,
    stdin=subprocess.PIPE,
    universal_newlines=True
)

with open("perft.epd") as fin:
    data = fin.readlines()

print "--------------------------------------------------------------------------------------------------------"    
print "| Depth |    Nodes   |                                   FEN                                  | STATUS |"
print "--------------------------------------------------------------------------------------------------------"
    
start = time.time()
    
for line in data:
    tokens = line.split(";")
    fen, last = tokens[0], tokens[-1]
    depth = int(last.split(" ")[0][1:])
    nodes = int(last.split(" ")[1])
    
    print "| {0:>5} | {1:>10} | {2:<70} | ".format(depth, nodes, fen),
    
    process.stdin.write("position fen %s\n" % (fen))
    process.stdin.write("perft %d\n" % (depth))
    process.stdin.flush()
    
    found = int(process.stdout.readline().strip())
    
    if found == nodes: print " PASS |"
    if found != nodes: print " FAIL | (%10d)" % (found)
    
end = time.time()

print "--------------------------------------------------------------------------------------------------------"

process.stdin.write("quit")
process.stdin.flush()

print "Total Time: %dms" % (1000 * (end - start))
