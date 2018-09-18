all: test_protocol

test_protocol: test_protocol.cpp waggle.h
	c++ -o test_protocol test_protocol.cpp

test: test_protocol
	./test_protocol
