import os
import sys
import random

f = open('inputfile', 'w')
# write 50 lines of random 128 bit numbers 
for i in range(50):
    # a line of 128 bits 
    line = ""
    for j in range(128):
        line = line + str(random.randint(0, 1))
    f.write(line + "\n")
f.close()
