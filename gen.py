import random
import sys

R = int(sys.argv[1])
C = int(sys.argv[2])

print R,C
for r in range(R):
  for c in range(C):
    if r == 0 and c == 0:
      ch = '.'
    else:
      ch = random.choice('BGMORY')
    sys.stdout.write(ch)
  sys.stdout.write('\n')
