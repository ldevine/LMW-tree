CC = /usr/local/bin/g++-4.8
INC_PATH =  
CFLAGS = -std=c++0x -O2 -march=native -mtune=native $(INC_PATH)

all: kmsig gensig


kmsig: KMeansSig.cpp
	$(CC) -o $@ $^ $(CFLAGS)

gensig: GenSig.cpp
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean cleanest kmeanssig gensig


clean:
	rm -f *.o

cleanest: clean
	rm -f kmsig
	rm -f gensig

