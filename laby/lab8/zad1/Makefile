CXX=gcc
CFLAGS=-Wall -std=c17

.PHONY: all clean

all: user printer

user: user.c
	$(CXX) $(CFLAGS) -c user.c
	$(CXX) $(CFLAGS) -o user user.c

printer: printer.c
	$(CXX) $(CFLAGS) -c printer.c
	$(CXX) $(CFLAGS) -o printer printer.c

clean:
	rm *.o user printer
