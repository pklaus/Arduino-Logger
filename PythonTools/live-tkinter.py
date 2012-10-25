#!/usr/bin/env python

import numpy as np
import matplotlib.pyplot as plt
import time
from Tkinter import *

master = Tk()

f = open('values.txt')

v = StringVar()
w = Label(master, textvariable=v, height=16).pack()

X = [[],[],[],[]]

def update():
    global v, f, X, master

    line = f.readline()
    if line:
        for i, value in enumerate(line.split()):
            X[i].append(value)
        print("Would plot the values now (lists currently containing %d entries)." % len(X[0]))
        v.set(str(X[0][-1]))
        master.after(1, update)
    else:
        master.after(50, update)

master.after(50, update)
master.mainloop()
