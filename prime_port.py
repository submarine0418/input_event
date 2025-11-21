import serial
import time
import sys

port = sys.argv[1]
print(f"Priming {port}...")

# Open port at 9600, disable flow control
ser = serial.Serial(port, 9600, rtscts=False, dsrdtr=False)

# Toggle DTR to reset Arduino (optional, but good for sync)
ser.dtr = False
time.sleep(0.1)
ser.dtr = True
time.sleep(2) # Wait for bootloader

print("Port primed. Closing...")
ser.close()