CC=c++ -I../src

all: base64_test

base64_test: base64_test.cpp ../src/Waggle.h
	$(CC) -o base64_test base64_test.cpp

test_protocol: test_protocol.cpp
	$(CC) -o test_protocol test_protocol.cpp

test: test_protocol
	./test_protocol
