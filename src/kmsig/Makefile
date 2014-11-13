CC = g++
INC_PATH = . 
CFLAGS = -std=c++0x -O2 -march=native -mtune=native $(INC_PATH)

all: kmeanssig


kmeanssig: KMeansSig.cpp
	$(CC) -o $@ $^ $(CFLAGS)


.PHONY: clean cleanest kmeanssig


clean:
	rm -f *.o

cleanest: clean
	rm -f kmeanssig
