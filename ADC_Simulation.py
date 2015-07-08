#!/usr/bin/env python

### Script for Python
### Helps to size the serial reference resistors when using
### an NTC to measure temperatures with an ADC.

from __future__ import division

from matplotlib import pyplot as plt
import numpy as np
from munch import Munch

import pdb; pdb.set_trace()

# Properties of the NTC:
#NTC = dict(B=3625, R0 = 10000, T0 = 25+273) # Buerklin NTC thermistor 10K, 3625K, 0.75 mW/K, B57550G1103F005
NTC = Munch(B=3977, R0 = 10000, T0 = 25+273)  # Reichelt NTC-0,2 10K

# Properties of the measurement circuit:
#Rref =    500; Uvcc = 4.6; ADCbits = 10; # Arduino / Ausheizlogger
#Rref =  10000; Uvcc =  5.; ADCbits = 10; # Arduino / Raumtemperatur-Logger
Rref =   50000; Uvcc =  5.; ADCbits = 10; # Arduino / Cooling Station

# Map the measurement range of the ADC:
#Um = range(0., Uvcc, Uvcc / /(2**ADCbits-1.))
Um = np.linspace(0., Uvcc, num=(2**ADCbits-1))
# remove the first and last element:
Um = Um[1:-1]

# Calculate the distinguishable resistance values of the NTC:
Rt = Rref*(Uvcc / Um - 1)    # by measuring the voltage accross the ref res.
#Rt = Rref / (Uvcc / Um - 1) # measuring the voltage across the NTC

# Calculate the temperature values belonging to those NTC resistances:
T = 1./(np.log(Rt/NTC.R0)/NTC.B + 1/NTC.T0 ) - 273.15

# Plot the temperatures versus their differences
plt.scatter(T[:-1], np.diff(T))
plt.show()

# Plot the Temperatures vs. discrete ADC values
#scatter([1:2^ADCbits-2],T)
