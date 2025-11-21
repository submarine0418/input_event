savedcmd_/home/pi/input_event/serial_wasd_ldisc.mod := printf '%s\n'   serial_wasd_ldisc.o | awk '!x[$$0]++ { print("/home/pi/input_event/"$$0) }' > /home/pi/input_event/serial_wasd_ldisc.mod
