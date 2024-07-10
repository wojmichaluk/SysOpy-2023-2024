CXX = gcc
CFLAGS = -Wall -std=c17

all: santa
	
santa: santa.c
	$(CXX) $(CFLAGS) santa.c -o santa -lpthread
	
clean:
	rm -f santa
