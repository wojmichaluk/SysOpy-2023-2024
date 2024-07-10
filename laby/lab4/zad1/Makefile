CXX=gcc
CFLAGS=-Wall -std=c17

.PHONY: all clean

all: descendant

descendant: descendant.c
	$(CXX) $(CFLAGS) -c descendant.c
	$(CXX) $(CFLAGS) -o descendant descendant.c

clean:
	rm *.o descendant
