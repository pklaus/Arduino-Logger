#!/usr/bin/env python
# -*- encoding: UTF8 -*-

# Author: Philipp Klaus, philipp.l.klaus AT web.de


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


# Some serial code from https://github.com/gregpinero/ArduinoPlot/blob/master/Arduino_Monitor.py

# example how to use the avrnetio class

import serial
## for sys.exit(1)
import sys
##for OptionParser() (see <http://optik.sourceforge.net/>)
from optparse import OptionParser
## for time.sleep()
import time
from electronics import Ntc
from Queue import Queue
from threading import Thread

DEFAULTS = {
    'serial_port': "/dev/tty.usbmodemfd121",
    #'serial_port': "/dev/tty.usbmodemfa131",
    'filename' : "./log_data.dat",
    'buffer_mode' : 1, # -1: system default, 0: unbuffered 1: line buffered
    'reference_voltage' : 4.36,
    'ADC_bit_depth': 10,
    'NTC_R0' : 10000.,
    'NTC_T0' : 25.+273,
    'NTC_B0' : 3625.,
    'R_voltage_divider' : 470.,
    'DAT_delimiter': ' ',
}

received_data = Queue()
closing = False
def receiving(ser):
    global received_data, closing
    buffer = ''
    while True:
        if closing: break
        buffer = buffer + ser.read(ser.inWaiting())
        if '\n' in buffer:
            lines = buffer.split('\n') # Guaranteed to have at least 2 entries
            if lines[-2]: received_data.put(lines[-2])
            buffer = lines[-1]
    if ser:
        ser.close()

def ADC_to_volt(ADC_values, ref_voltage, bit_depth):
    max_val = (2**bit_depth-1)
    return [float(val)/max_val * ref_voltage for val in ADC_values]

def main():
    global received_data, closing

    parser = OptionParser()
    parser.add_option("-p", "--serial-port",
                    action="store", type="string", dest="serial_port",
                    help="Read from serial port COMx", metavar="COMx")
    parser.add_option("-f", "--file",
                    action="store", type="string", dest="filename",
                    help="write data to FILE", metavar="FILE")
    parser.add_option("-r", "--reference-voltage",
                    action="store", type="float", dest="reference_voltage",
                    help="VOLTAGE used as ADC reference value", metavar="VOLTAGE")
    parser.add_option("-q", "--quiet",
                    action="store_true", dest="quiet", default=0,
                    help="only print status messages to stdout")

    (options, args) = parser.parse_args()

    if options.serial_port == None:
        options.serial_port = DEFAULTS['serial_port']

    if options.filename == None:
        options.filename = DEFAULTS['filename']
        if not options.quiet: print "Logging to " + DEFAULTS['filename'] + " as no filenmame was supplied on the command line."

    if options.reference_voltage == None:
        options.reference_voltage = DEFAULTS['reference_voltage']

    if not options.quiet: print "Writing to the file: %s" % options.filename
    logfile = open(options.filename, 'a', DEFAULTS['buffer_mode'])

    ser = None
    ntc = Ntc(DEFAULTS['NTC_R0'],DEFAULTS['NTC_T0'],DEFAULTS['NTC_B0'])
    try:
        ser = serial.Serial(
            port=options.serial_port,
            baudrate=9600,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.1,
            xonxoff=0,
            rtscts=0,
            interCharTimeout=None
        )
        Thread(target=receiving, args=(ser,)).start()
        while 1:
            if received_data.empty(): continue
            line = received_data.get()
            ADCs = line.split()
            ADCsInVolts = ADC_to_volt(ADCs, options.reference_voltage, DEFAULTS['ADC_bit_depth'])
            ## care for 0.0 values of the voltage:
            ADCsInVolts = [0.000001 if voltage == 0 else voltage for voltage in ADCsInVolts]
            resistances = [ DEFAULTS['R_voltage_divider'] * (( DEFAULTS['reference_voltage']/voltage)-1.)  for voltage in ADCsInVolts ]
            ## care for 0.0 values of the resistance:
            resistances = [ 1e-10 if resistance <= 0. else resistance for resistance in resistances]
            Temperatures = [ ntc.ntc_resitance_to_temp(resistance) for resistance in resistances ]
            for ADC in ADCs :
                logfile.write("%s " % (ADC))
            for ADCInVolts in ADCsInVolts :
                logfile.write("%.4f " % (ADCInVolts))
            for temperature in Temperatures:
                logfile.write("%.1f " % (temperature))
            logfile.write("\n")
            print DEFAULTS['DAT_delimiter'].join(['%.2f' % temperature for temperature in Temperatures])
            time.sleep(0.1)

    except KeyboardInterrupt:
        if not options.quiet: print "[Ctrl]-[C] pressed: closing logfile."
        closing = True
    except serial.serialutil.SerialException, e:
        #no serial connection
        print("Could not connect. Reason:" + str(e))
        closing = True
    except Exception, e:
        print("Some other error occured: %s" % e)
        closing = True
        raise e

    if logfile:
        logfile.close()

if __name__ == '__main__':
    main()
    #print ADC_to_volt([1,4,100], 4.6, 10)
