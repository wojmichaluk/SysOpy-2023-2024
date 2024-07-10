CXX=gcc
CFLAGS=-Wall -std=c17

.PHONY: all clean

all: sender catcher

sender: sender.c
	$(CXX) $(CFLAGS) -c sender.c
	$(CXX) $(CFLAGS) -o sender sender.c

catcher: catcher.c
	$(CXX) $(CFLAGS) -c catcher.c
	$(CXX) $(CFLAGS) -o catcher catcher.c 

clean:
	rm *.o sender catcher
