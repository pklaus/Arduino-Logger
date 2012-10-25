#!/usr/bin/env python

import pylab
from pylab import *
import time

f = open('serial-dump.txt')

# skip (old) stuff we don't want
f.seek(-200,2) # go to (almost) the end of the file
f.readline() # go to next line break

X = [[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[]]

i = 0

maximum = 1024.
max_update_rate = 4 # maximum update rate in Hz

xAchse=pylab.arange(0,100,1)
yAchse=pylab.array([0]*100)

fig = pylab.figure(1)
ax = fig.add_subplot(111)
ax.grid(True)
ax.set_title("Realtime Values Plot")
ax.set_xlabel("Time")
ax.set_ylabel("Value")
ax.axis([0,100,0.,maximum])
line1=ax.plot(xAchse,yAchse,'-')

manager = pylab.get_current_fig_manager()

values = [0 for x in range(100)]


def ValueGetter(arg):
    global values, f

    line = f.readline()
    if line:
        for i, value in enumerate(line.split()):
            if i == 0: values.append(value)

lastUpdate = 0.0
def RealtimePloter(arg):
  global values, lastUpdate
  if time.time() - lastUpdate < 1./max_update_rate: return
  lastUpdate = time.time()
  CurrentXAxis=pylab.arange(len(values)-100,len(values),1)
  line1[0].set_data(CurrentXAxis,pylab.array(values[-100:]))
  ax.axis([CurrentXAxis.min(),CurrentXAxis.max(),0.,maximum])
  manager.canvas.draw()
  #manager.show()

timer = fig.canvas.new_timer(interval=20)
timer.add_callback(RealtimePloter, ())
timer2 = fig.canvas.new_timer(interval=20)
timer2.add_callback(ValueGetter, ())
timer.start()
timer2.start()

pylab.show()
