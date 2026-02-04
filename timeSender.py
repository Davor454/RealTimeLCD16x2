import serial
from datetime import datetime
import time

ser = serial.Serial("COM5", 115200, timeout=1) #Change "COM5" to your actual COM port.
time.sleep(2)

now = datetime.now()
msg = now.strftime("T:%Y-%m-%d %H:%M:%S\n")

ser.write(msg.encode())
print("Sent:", msg)

ser.close()
