import math
import os
import os.path
import re
import subprocess
import sys

colour_map = [
  ('.', -1),
  ('B', 0),
  ('G', 1),
  ('M', 2),
  ('O', 3),
  ('R', 4),
  ('Y', 5),
  ]

def to_colour(x):
  for c,y in colour_map:
    if y+1 == x:
      return c
  raise ValueError('Unknown colour %d' % x)

if os.path.exists('floodit_peter_stats.csv'):
  os.remove('floodit_peter_stats.csv')

i = 0
for line in file('solver_stats_sorted.csv'):
  m = re.search('"([0-6]+)"', line)
  if m is None: continue

  game_as_string = m.group(1)
  sz = len(game_as_string)
  n = int(math.sqrt(sz))
  if n*n != sz:
    raise ValueError()

  f = file('floodit.tmp.in', 'w')
  print >>f, n, n
  for r in range(n):
    for c in range(n):
      f.write(to_colour(int(game_as_string[r*n + c])))
    f.write('\n')
  f.close()

  subprocess.call('./floodit < floodit.tmp.in > floodit.tmp.out', shell=True)

  i += 1
  print i
