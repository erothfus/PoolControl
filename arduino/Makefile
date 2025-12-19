#
# Makefile
#
#   Used to both configure programming of the pool control
#   arduino Nano, and to update its program.
#
#   You can also use this to fire-up a serial monitor for
#   the board.
#
#   NOTES - the board is assumed to be directly connected
#   to the RPi, and WE are the RPi. 

CLI := ~/bin/arduino-cli
PORT := /dev/ttyUSB0
FQBN := arduino:avr:nano
SKETCH := PoolControl
#LEVEL := info
LEVEL := --log-level debug

list:
	$(CLI) board list

monitor:
	$(CLI) $(LEVEL) monitor -p $(PORT)

compile:
	$(CLI) compile --fqbn $(FQBN) $(SKETCH)

upload:
	$(CLI) $(LEVEL) upload -p $(PORT) --fqbn $(FQBN) $(SKETCH)

init:
	$(CLI) core update-index
	$(CLI) board list
	$(CLI) core install arduino:avr

