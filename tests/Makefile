CC=c++ -I../src

all: test_protocol

test_protocol: test_protocol.cpp
	$(CC) -o test_protocol test_protocol.cpp

test: test_protocol
	./test_protocol
