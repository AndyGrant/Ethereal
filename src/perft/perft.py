# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#                                                                             #
#   Ethereal is a UCI chess playing engine authored by Andrew Grant.          #
#   <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>          #
#                                                                             #
#   Ethereal is free software: you can redistribute it and/or modify          #
#   it under the terms of the GNU General Public License as published by      #
#   the Free Software Foundation, either version 3 of the License, or         #
#   (at your option) any later version.                                       #
#                                                                             #
#   Ethereal is distributed in the hope that it will be useful,               #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of            #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             #
#   GNU General Public License for more details.                              #
#                                                                             #
#   You should have received a copy of the GNU General Public License         #
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.     #
#                                                                             #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

import argparse, subprocess, sys, time

parser = argparse.ArgumentParser()
parser.add_argument('engine',  help='Path to Engine')
parser.add_argument('dataset', help='PERFT data file')
parser.add_argument('--depth', help='Depth of PERFT [ blank = unlimited ]', default=128)
arguments = parser.parse_args()

process = subprocess.Popen(
    arguments.engine,
    stderr=subprocess.STDOUT,
    stdout=subprocess.PIPE,
    stdin=subprocess.PIPE,
    universal_newlines=True
)

with open(arguments.dataset) as fin:
    data = fin.readlines()

SEPERATOR = '-' * 111 + '\n'
HEADER    = '| {0:^5} | {1:^11} | {2:^76} | {3:^6} |\n'

sys.stdout.write(SEPERATOR)
sys.stdout.write(HEADER.format('Depth', 'Nodes', 'FEN', 'Status'))
sys.stdout.write(SEPERATOR)

start = time.time()

for line in data:

    for token in line.split(';')[1:]:

        fen   = line.split(';')[0]
        depth = int(token.split(' ')[0][1:])
        nodes = int(token.split(' ')[1])
        if depth > arguments.depth: break

        sys.stdout.write('| {0:>5} | {1:>11} | {2:<76} | '.format(depth, nodes, fen))
        sys.stdout.flush()

        process.stdin.write('position fen %s\n' % (fen))
        process.stdin.write('perft %d\n' % (depth))
        process.stdin.flush()

        found = int(process.stdout.readline().strip())

        if found == nodes: sys.stdout.write (' PASS  |\n')
        if found != nodes: sys.stdout.write (' FAIL  | (%10d)\n' % (found))

process.stdin.write('quit')
process.stdin.flush()

sys.stdout.write(SEPERATOR)
sys.stdout.write('Total Time: %dms' % (1000 * (time.time() - start)))
