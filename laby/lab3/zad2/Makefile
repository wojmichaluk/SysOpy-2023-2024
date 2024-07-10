CXX=gcc
CFLAGS=-Wall -std=c17

.PHONY: all clean

all: catalog_overview

catalog_overview: catalog_overview.c
	$(CXX) $(CFLAGS) -c catalog_overview.c
	$(CXX) $(CFLAGS) -o catalog_overview catalog_overview.c

clean:
	rm *.o catalog_overview
