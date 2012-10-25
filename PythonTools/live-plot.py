#!/usr/bin/env python

import time

f = open('values.txt')

# skip (old) stuff we don't want
f.seek(-200,2) # go to (almost) the end of the file
f.readline() # go to next line break

X = [[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[]]

while True:
    line = f.readline()
    if line:
        for i, value in enumerate(line.split()):
            X[i].append(value)
            print i, value
        #print("Would plot the values now (lists currently containing %d entries)." % len(X[0]))
    else:
        time.sleep(0.001)
