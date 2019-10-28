# CC=c++ -std=c++11 -I../src
CC=c++ -I../src

test: waggle_test
	./waggle_test

waggle_test: waggle_test.cpp ../src/waggle.h
	$(CC) -o waggle_test waggle_test.cpp
