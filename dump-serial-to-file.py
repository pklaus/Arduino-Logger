#!/usr/bin/env python
# -*- encoding: UTF8 -*-

# Author: Philipp Klaus, philipp.l.klaus AT web.de


#   This file is part of ardu-measure.
#
#   ardu-measure is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   ardu-measure is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with ardu-measure.  If not, see <http://www.gnu.org/licenses/>.

import serial

DEFAULTS = {
    'serial_port': "/dev/tty.usbserial-FTGACCA2",
    'filename' : "./serial-dump.txt",
    'buffer_mode' : 1, # -1: system default, 0: unbuffered 1: line buffered
}

import serial
import sys
import time

def main():
    port = serial.Serial(DEFAULTS['serial_port'], 9600)

    logfile = open(DEFAULTS['filename'], 'a', DEFAULTS['buffer_mode'])
    buffer = ''
    closing = False
    while True:
        if closing: break
        try:
            buffer = buffer + port.read(port.inWaiting())
            if '\n' in buffer:
                lines = buffer.split('\n') # Guaranteed to have at least 2 entries
                if lines[-2]:
                    print lines[-2]
                    logfile.write(lines[-2]+'\n')
                buffer = lines[-1]
            else:
                time.sleep(0.02)

        except KeyboardInterrupt:
            print "[Ctrl]-[C] pressed: closing logfile."
            closing = True
        except serial.serialutil.SerialException, e:
            #no serial connection
            print("Could not connect. Reason:" + str(e))
            closing = True

    if logfile:
        logfile.close()
    if port:
        port.close()

if __name__ == '__main__':
    main()
