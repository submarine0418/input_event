# Linux Kernel TTY Line Discipline Driver for Arduino Joystick

![License](https://img.shields.io/badge/license-GPL-blue.svg)
![Kernel](https://img.shields.io/badge/kernel-6.12+-green.svg)
![Platform](https://img.shields.io/badge/platform-Raspberry%20Pi-red.svg)

A custom Linux kernel TTY Line Discipline (LDISC #29) that transforms Arduino joystick serial input into native Linux keyboard events. This enables hardware-level input device integration without requiring user-space drivers or scripts.

## 🎯 Features

- **Zero-Latency Input**: Kernel-space processing for real-time response
- **Native Integration**: Works seamlessly with X11, Wayland, terminals, and games
- **Hot-Pluggable**: Dynamic attachment using standard `ldattach` utility
- **Production-Ready**: Includes proper memory management, spinlock protection, and disconnect handling
- **Modern API**: Updated for Linux Kernel 6.12+ TTY subsystem

## 🛠️ System Architecture
```
Arduino Joystick → UART/USB → TTY Driver → Line Discipline #29 → Input Subsystem → Userspace
                                              ↑
                                         (This Module)
```

### How It Works

1. **Arduino** sends serial commands (e.g., `W1\n` for W key press)
2. **TTY Line Discipline** intercepts and parses the serial stream
3. **Input Subsystem** converts parsed data to standard Linux input events
4. **Kernel** delivers events to all listening applications (games, terminals, etc.)

## 📋 Prerequisites

### Hardware
- Raspberry Pi (or any Linux system with kernel 6.x+)
- Arduino (Uno, Nano, or compatible)
- Analog joystick module

### Software
- Linux kernel 6.12 or later
- Kernel headers (`linux-headers-$(uname -r)`)
- Build essentials (`build-essential`)

## 🚀 Quick Start

### 1. Build the Module
```bash
# Clone the repository
git clone https://github.com/submarine0418/input_event.git


# Compile
make

# Install (optional, for persistence)
sudo make install
```

### 2. Load the Module
```bash
# Load into kernel
sudo insmod serial_wasd_ldisc.ko

# Verify it's loaded
lsmod | grep serial_wasd
```

### 3. Connect Arduino

Upload the Arduino sketch to your board:
```cpp
// Arduino code (simplified)
void setup() {
  Serial.begin(9600);
}

void loop() {
  int x = analogRead(A0);
  
  if (x > 700) {
    Serial.println("W1");  // W key pressed
  } else {
    Serial.println("W0");  // W key released
  }
  
  Serial.println(".");  // Heartbeat
  delay(100);
}
```

### 4. Attach Line Discipline
```bash
# Find your Arduino's port
ls /dev/ttyACM*  # or /dev/ttyUSB*

# Attach the driver (keep terminal open)
sudo ldattach -d -s 9600 29 /dev/ttyACM0
```

### 5. Test
```bash
# Monitor kernel logs
dmesg -w

# Test input events
sudo evtest
# Select the "serial-wasd" device and move your joystick
```

## 📡 Serial Protocol

The Arduino must send newline-terminated commands at **9600 baud**:

| Command | Meaning          |
|---------|------------------|
| `W1\n`  | W key pressed    |
| `W0\n`  | W key released   |
| `A1\n`  | A key pressed    |
| `A0\n`  | A key released   |
| `S1\n`  | S key pressed    |
| `S0\n`  | S key released   |
| `D1\n`  | D key pressed    |
| `D0\n`  | D key released   |
| `.\n`   | Heartbeat (keepalive) |


## 🏗️ Technical Details

### Key Implementation Points

1. **Interrupt Context Safety**: `receive_buf()` runs in interrupt context and uses spinlocks
2. **Termios Configuration**: Automatically sets raw mode and CLOCAL flag
3. **Buffer Management**: Implements line buffering to handle partial reads
4. **Input Device Registration**: Creates a virtual keyboard (`EV_KEY` type)

### Critical Code Sections

#### Setting Receive Buffer Size
```c
tty->receive_room = 65536;  // Allow kernel to push data
```

#### Forcing Raw Mode
```c
tty_set_termios(tty, &tty->termios);  // Apply CLOCAL immediately
```

#### Event Reporting
```c
input_report_key(dev->input, KEY_W, state);
input_sync(dev->input);
```

## 📝 Files
```
.
├── serial_wasd_ldisc.c    # Main driver source
├── Makefile               # Build configuration
├── README.md              # This file
└── arduino/
    └── joystick.ino       # Arduino sketch (optional)
```

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

### Development Setup
```bash
# Enable debug output
# Uncomment pr_info lines in serial_wasd_ldisc.c

# Rebuild and test
make clean && make
sudo rmmod serial_wasd_ldisc
sudo insmod serial_wasd_ldisc.ko
```

## 📚 Resources

- [Linux TTY Subsystem Documentation](https://www.kernel.org/doc/html/latest/driver-api/tty/index.html)
- [Input Subsystem Guide](https://www.kernel.org/doc/html/latest/input/input.html)
- [Line Disciplines Overview](https://www.kernel.org/doc/html/latest/driver-api/tty/tty_ldisc.html)

## 📄 License

This project is licensed under the GNU General Public License v2.0 (GPL-2.0).

See the Linux kernel's [COPYING](https://www.kernel.org/doc/html/latest/process/license-rules.html) file for details.

## 👤 Author

**Austin Chen (陳彥廷)**
- GitHub: [@submarine0418](https://github.com/submarine0418)
- Email: austin770115@gmail.com.com

---

⭐ If you find this project useful, please consider giving it a star!

💡 **Use Case**: This driver was developed as part of a research project for real-time parameter adjustment of underwater image enhancement algorithms (DCP dehazing) at National Taiwan University.
