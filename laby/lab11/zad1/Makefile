CXX=gcc
CFLAGS=-Wall -std=c17

.PHONY: all clean

all: server client

server: server.c common.h
	$(CXX) $(CFLAGS) -c server.c
	$(CXX) $(CFLAGS) -o server server.c -lpthread

client: client.c common.h
	$(CXX) $(CFLAGS) -c client.c
	$(CXX) $(CFLAGS) -o client client.c -lpthread

clean:
	rm *.o server client
