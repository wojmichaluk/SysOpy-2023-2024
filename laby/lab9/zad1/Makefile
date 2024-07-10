CXX=gcc
CFLAGS=-Wall

all: life
	
life: grid.c life.c
	$(CXX) $(CFLAGS) grid.c life.c -o life -lncursesw -lpthread
	
clean:
	rm -f life
