import serial
import uinput
import time

# Setup Virtual Keyboard
try:
    device = uinput.Device([uinput.KEY_W, uinput.KEY_A, uinput.KEY_S, uinput.KEY_D])
except OSError:
    print("Error: Run with sudo!")
    exit(1)

PORT = '/dev/ttyACM0'  # Update this if it changes!

print(f"Connecting to {PORT}...")

while True:
    try:
        with serial.Serial(PORT, 9600, timeout=1) as ser:
            ser.dtr = False; time.sleep(1); ser.dtr = True; time.sleep(2)
            print("Connected! Move joystick.")
            
            while True:
                if ser.in_waiting:
                    try:
                        line = ser.readline().decode('utf-8', errors='ignore').strip()
                        if len(line) >= 2:
                            key = line[0]
                            val = 1 if line[1] == '1' else 0
                            
                            if key == 'W': device.emit(uinput.KEY_W, val)
                            elif key == 'A': device.emit(uinput.KEY_A, val)
                            elif key == 'S': device.emit(uinput.KEY_S, val)
                            elif key == 'D': device.emit(uinput.KEY_D, val)
                            # print(f"Key {key} {val}") 
                    except ValueError: pass
    except Exception as e:
        print(f"Waiting for device... ({e})")
        time.sleep(2)