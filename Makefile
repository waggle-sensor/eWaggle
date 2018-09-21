all: test_protocol

test_protocol: test_protocol.cpp waggle
	c++ -o test_protocol test_protocol.cpp

test: test_protocol
	./test_protocol
