CXX=gcc
CFLAGS=-Wall -std=c17

.PHONY: all clean

all: reverse

reverse: reverse.c
	$(CXX) $(CFLAGS) -c reverse.c
	$(CXX) $(CFLAGS) -o reverse reverse.c

clean:
	rm *.o reverse
