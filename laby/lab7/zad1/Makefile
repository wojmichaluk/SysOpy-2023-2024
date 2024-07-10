CXX=gcc
CFLAGS=-Wall -std=c17

.PHONY: all clean

all: server client

server: server.c
	$(CXX) $(CFLAGS) -c server.c
	$(CXX) $(CFLAGS) -o server server.c

client: client.c
	$(CXX) $(CFLAGS) -c client.c
	$(CXX) $(CFLAGS) -o client client.c

clean:
	rm *.o server client
