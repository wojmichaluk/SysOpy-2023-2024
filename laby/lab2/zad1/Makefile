CXX=gcc
CFLAGS=-Wall -std=c17

.PHONY: all clean

all: static shared dynamic

libraries:
	$(CXX) $(CFLAGS) -c library.c
#utworzenie biblioteki dynamicznej z rozszerzeniem .so
	$(CXX) $(CFLAGS) -shared library.o -o liblibrary.so

static: libraries
#utworzenie biblioteki statycznej z rozszerzeniem .a
	ar rcs liblibrary.a library.o
#utworzenie programu 
	$(CXX) main_static.c liblibrary.a $(CFLAGS) -o main_static

shared: libraries
	$(CXX) $(CFLAGS) -fPIC -c library.c
#dodatkowa ścieżka
	$(CXX) main_shared.c $(CFLAGS) -L. -llibrary -Wl,-rpath=. -o main_shared

dynamic: libraries
	$(CXX) main_dynamic.c $(CFLAGS) -o main_dynamic -ldl

clean:
	rm *.o *.a *.so main_static main_shared main_dynamic