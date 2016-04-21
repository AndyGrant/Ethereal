# This script creates the KNIGHT_MAP[64][9] found
# In KnightMap.h. The map serves as a lookup table
# For possible moves, before accounting for Check
# And for capturing your own pieces

# Do not make any changes to this file, as putting
# an incorrect map into KnightMap.h breaks the Engine

def valid(x,y):
  return (0 <= x <= 7) and (0 <= y <= 7)

s = "{{2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2}}"
s = s.replace("{","[")
s = s.replace("}","]")
exec "k = " + s

m = [[] for f in range(64)]

for x in range(8):
  for y in range(8):
    for (dx,dy) in k:
      if valid(x+dx,y+dy):
        m[x*8+y].append(8*(x+dx) + y+dy)
        
for f in range(len(m)):
  for x in range(9-len(m[f])):
    m[f].append(-1)


s = ",\n\t".join([f.__str__() for f in m])
s = s.replace("[","{")
s = s.replace("]","}")
s = s + "\n};"
s =  "static int KNIGHT_MAP[64][9] = {\n\t" + s

print s