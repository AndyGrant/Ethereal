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

import argparse, subprocess, sys, time

parser = argparse.ArgumentParser()
parser.add_argument("ENGINE", help="Path to Engine")
parser.add_argument("DATASET", help="PERFT data file")
arguments = parser.parse_args()

process = subprocess.Popen(
    arguments.ENGINE,
    stderr=subprocess.STDOUT,
    stdout=subprocess.PIPE,
    stdin=subprocess.PIPE,
    universal_newlines=True
)

with open(arguments.DATASET) as fin:
    data = fin.readlines()

print("---------------------------------------------------------------------------------------------------------------")
print("| Depth |    Nodes    |                                      FEN                                     | STATUS |")
print("---------------------------------------------------------------------------------------------------------------")

start = time.time()

for line in data:
    tokens = line.split(";")
    fen, last = tokens[0], tokens[-1]
    depth = int(last.split(" ")[0][1:])
    nodes = int(last.split(" ")[1])

    sys.stdout.write("| {0:>5} | {1:>11} | {2:<76} | ".format(depth, nodes, fen)),
    sys.stdout.flush()

    process.stdin.write("position fen %s\n" % (fen))
    process.stdin.write("perft %d\n" % (depth))
    process.stdin.flush()

    found = int(process.stdout.readline().strip())

    if found == nodes: sys.stdout.write (" PASS  |\n")
    if found != nodes: sys.stdout.write (" FAIL  | (%10d)\n" % (found))

end = time.time()

print("---------------------------------------------------------------------------------------------------------------")

process.stdin.write("quit")
process.stdin.flush()

print("Total Time: %dms" % (1000 * (end - start)))
