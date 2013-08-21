CC = g++
CFLAGS = -std=c++0x -O4 -march=native -mtune=native
#CFLAGS = -std=c++0x -O0 -ggdb 
LDFLAGS = -L/opt/local/lib -lpthread -lboost_system-mt -lboost_thread-mt -lboost_timer-mt #-lrt

all: emtree

emtree: EMTree.o
	$(CC) -o $@ $^ $(LDFLAGS)

EMTree.o: src/EMTree.cpp
	$(CC) -c $(CFLAGS) $<

.PHONY: clean cleanest

clean:
	rm -f *.o

cleanest: clean
	rm -f emtree
