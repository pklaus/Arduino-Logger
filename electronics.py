#
# -*- encoding: UTF8 -*-

# author: Philipp Klaus, philipp.l.klaus AT web.de


#   This file is part of avrnetio.
#
#   avrnetio is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   avrnetio is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with avrnetio.  If not, see <http://www.gnu.org/licenses/>.


## for debugging (set debug mark with pdb.set_trace() )
#import pdb
## for math.log()
import math


# NTCs are temperature dependent resistors.
class Ntc(object):
    """ Class to calculate the temperature of a NTC thermistor ( see http://en.wikipedia.org/wiki/Thermistor )

     formula that has been used is also found here: http://en.wikipedia.org/wiki/Thermistor#B_parameter_equation
     better alternative, the Steinhart-Hart equation: http://en.wikipedia.org/wiki/Thermistor#Steinhart-Hart_equation

    setup of the NTC in your circuit

         U_VCC
         ---+
            #
            #  serial constant resistor R   (10k)
            #
            |---------> U_NTC 
            #
            #   NTC R_NTC  (4.7k)
            #
         ---+
         GND

    """

    def __init__(self, RN0=4700.0, TN0 = 25.0+273.0, B0 = 3977.0):
        """ custom constructor
        RN0: Resistance at temperature TN0,   B0: coefficient """
        self.RN0 = RN0
        self.TN0 = TN0
        self.B0 = B0
        # R = 10000 Ohm
        self.serial_resistance=10000.0
        # U_VCC = 5V
        self.Uvcc = 5.0

    # we want to calculate the temperature T at a measured resistance R of the NTC
    # http://tuxgraphics.org/common/images2/article07051/Ntcformula.gif
    # takes the value of an electrical resistance [Ohm] and gives back a temperature [K]
    def ntc_resitance_to_temp(self, resistance):
        temperature = 1/(math.log(resistance/self.RN0)/self.B0+1/self.TN0)
        return temperature

    # NTC voltage -> NTC resistance   for connected voltage dividers
    def calculate_resistance_of_ntc(self, Untc):
        # R_NTC   =   R / ( U_VCC / U_NTC - 1)
        if (Untc <= 0):
            # never divide by zero:
            Untc=0.001;
        ohm= self.serial_resistance / (( self.Uvcc / Untc ) - 1 );
        return ohm

    def ntc_potential_to_temp(self, potential):
        return self.ntc_resitance_to_temp(self.calculate_resistance_of_ntc(potential))


if __name__ == '__main__':
    # usage example:
    ntc = Ntc()
    res = 3000
    print("A resistance of %d Ohm of this NTC means a temperature of %.2f K." % (res, ntc.ntc_resitance_to_temp(res)))
