CXX=gcc
CFLAGS=-Wall -std=c17

.PHONY: all clean

all: parent_child

parent_child: parent_child.c
	$(CXX) $(CFLAGS) -c parent_child.c
	$(CXX) $(CFLAGS) -o parent_child parent_child.c

clean:
	rm *.o parent_child