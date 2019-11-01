all: micro.hex arduino_due_x.bin

installdep:
	arduino-cli core update-index
	arduino-cli core install arduino:avr
	arduino-cli core install arduino:sam

micro.hex: simple.ino ../../src/*
	arduino-cli compile -b arduino:avr:micro -o micro

arduino_due_x.bin: simple.ino ../../src/*
	arduino-cli compile -b arduino:sam:arduino_due_x -o arduino_due_x

clean:
	rm -f micro.* arduino_due_x.*
