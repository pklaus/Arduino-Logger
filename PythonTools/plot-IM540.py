#!/usr/bin/env python

import time

f = open('serial-dump.txt')

Y = [[],[],[],[],[],[],[],[]]

while True:
    line = f.readline()
    if line:
        for i, value in enumerate(line.split(',')):
            if i%2:
                value = float(value)
            Y[i].append(value)
            #print i, value
    else:
        #time.sleep(0.001)
        ## or
        break

import matplotlib.pyplot as plt
X = [i / 60. for i in range(len(Y[1]))]
N = 80
EMA, weight = [], 2./(N+1)
EMA.append(Y[1][0])
for i in range(1,len(Y[1])):
    EMA.append(weight * Y[1][i] + (1.-weight) * EMA[i-1])
fig = plt.figure()
plt.plot(X, Y[1], marker='s', markersize=1, linestyle='None', label='Measured Data Points')
plt.plot(X, EMA, linewidth=2.0, label='Exponential Moving Average Filter (N=%d)' % N)
plt.xlabel('Minutes')
plt.ylabel('Pressure / mbar')
plt.title('Pressure Reading from Vacuuum Controller IM 540')
plt.legend()
plt.show()
fig.savefig('./plot.png', dpi=600)
fig.savefig('./plot.pdf')
