CXX=gcc
CFLAGS=-Wall -std=c17

.PHONY: all clean

all: reaction

reaction: reaction.c
	$(CXX) $(CFLAGS) -c reaction.c
	$(CXX) $(CFLAGS) -o reaction reaction.c

clean:
	rm *.o reaction
