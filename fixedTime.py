import serial
from datetime import datetime, time
import time as time_module

ser = serial.Serial("COM3", 115200, timeout=1)  # Change "COM3" to your actual COM port
time_module.sleep(2)

# Set the time to 23:59:59
fixed_time = time(23, 59, 55)
# Create a datetime object with today's date and fixed time
now = datetime.combine(datetime.today(), fixed_time)

msg = now.strftime("T:%Y-%m-%d %H:%M:%S\n")
ser.write(msg.encode())
print("Sent:", msg)

ser.close()