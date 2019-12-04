CC=c++ -Wall -std=c++11 -I../src

test: waggle_test
	./waggle_test

waggle_test: waggle_test.cpp ../src/*
	$(CC) -o waggle_test waggle_test.cpp
