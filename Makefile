.PHONY: all
all: compile upload

.PHONY: compile
compile:
	arduino-cli compile \
		-b arduino:avr:nano \
		eeprom_programming

.PHONY: upload
upload:
	arduino-cli upload \
		-b arduino:avr:nano \
		-p /dev/cu.usbserial-1440 \
		eeprom_programming

.PHONY: monitor
monitor:
	arduino-cli monitor \
		-p /dev/cu.usbserial-1440 \
		-c baudrate=115200
