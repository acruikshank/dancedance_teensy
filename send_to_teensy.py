import serial
import sys

with serial.Serial('/dev/tty.usbmodem611591', 9600, timeout=1) as ser1:
  with serial.Serial('/dev/tty.usbmodem611661', 9600, timeout=1) as ser2:
    values = bytearray([int(sys.argv[1])])
    ser1.write(values)
    ser2.write(values)
