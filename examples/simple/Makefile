all: importlib micro.hex wagman.bin

importlib:
	cp ../../src/* .

micro.hex: simple.ino ../../src/*
	arduino-cli compile -b arduino:avr:micro -o micro

wagman.bin: simple.ino ../../src/*
	arduino-cli compile -b arduino:sam:arduino_due_x -o wagman
