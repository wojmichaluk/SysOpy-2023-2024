CXX=gcc
CFLAGS=-Wall -std=c17

.PHONY: all clean

all: unnamed_pipe

unnamed_pipe: unnamed_pipe.c
	$(CXX) $(CFLAGS) -c unnamed_pipe.c
	$(CXX) $(CFLAGS) -o unnamed_pipe unnamed_pipe.c -lm

clean:
	rm *.o unnamed_pipe
