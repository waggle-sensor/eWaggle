all: test_protocol

test_protocol: test_protocol.cpp waggle/waggle.h
	c++ -o test_protocol test_protocol.cpp

test: test_protocol
	./test_protocol
