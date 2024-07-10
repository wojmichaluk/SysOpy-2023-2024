CXX=gcc
CFLAGS=-Wall -std=c17

all: countdown

countdown: countdown.c
	$(CXX) $(CFLAGS) -c countdown.c
	$(CXX) $(CFLAGS) -g -o countdown countdown.c

.PHONY: clean

clean: 
	rm *.o countdown