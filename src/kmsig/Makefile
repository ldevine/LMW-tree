CC = /usr/local/bin/g++-4.8
INC_PATH =  
CFLAGS = -std=c++11 -O2 -march=native -mtune=native $(INC_PATH)
LIBS = -lpthread
LDFLAGS = $(LIBS)

all: kmsig gensig


kmsig: KMeansSig.cpp
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

gensig: GenSig.cpp
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.PHONY: clean cleanest kmeanssig gensig


clean:
	rm -f *.o

cleanest: clean
	rm -f kmsig
	rm -f gensig

