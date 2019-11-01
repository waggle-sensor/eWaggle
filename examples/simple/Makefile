all: micro.hex arduino_due_x.bin

installdep:
	arduino-cli core update-index
	arduino-cli core install arduino:avr
	arduino-cli core install arduino:sam

micro.hex: simple.ino ../../src/*
	mkdir -p firmware
	arduino-cli compile -b arduino:avr:micro -o firmware/micro

arduino_due_x.bin: simple.ino ../../src/*
	mkdir -p firmware
	arduino-cli compile -b arduino:sam:arduino_due_x -o firmware/arduino_due_x

clean:
	rm -rf firmware/
