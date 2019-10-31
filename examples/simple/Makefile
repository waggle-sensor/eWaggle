all: importlib micro.hex wagman.bin

installdep:
	arduino-cli core update-index
	arduino-cli core install arduino:avr
	arduino-cli core install arduino:sam

importlib:
	cp ../../src/* .

micro.hex: simple.ino ../../src/*
	arduino-cli compile -b arduino:avr:micro -o micro

wagman.bin: simple.ino ../../src/*
	arduino-cli compile -b arduino:sam:arduino_due_x -o wagman
