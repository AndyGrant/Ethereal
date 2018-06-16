# Ethereal is a UCI chess playing engine authored by Andrew Grant.
# <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>
#
# Ethereal is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Ethereal is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
