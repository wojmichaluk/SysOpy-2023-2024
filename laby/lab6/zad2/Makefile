CXX=gcc
CFLAGS=-Wall -std=c17

.PHONY: all clean

all: named_pipe worker

named_pipe: named_pipe.c
	$(CXX) $(CFLAGS) -c named_pipe.c
	$(CXX) $(CFLAGS) -o named_pipe named_pipe.c

worker: worker.c
	$(CXX) $(CFLAGS) -c worker.c
	$(CXX) $(CFLAGS) -o worker worker.c

clean:
	rm *.o named_pipe worker
