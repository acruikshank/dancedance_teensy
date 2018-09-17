import serial
import sys

ids = ['4931040','4933780','4933320','4933800','4919670','4931340','4933500']
connections = [serial.Serial('/dev/serial/by-id/usb-Teensyduino_USB_Serial_'+id+'-if00', 115200, timeout=1) for id in ids]

for i, conn in enumerate(connections):
  values = bytearray([0, 8*i])
  conn.write(values)


values = bytearray([int(sys.argv[1]), int(sys.argv[2])])
for conn in connections:
  conn.write(values)
