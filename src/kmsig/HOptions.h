#ifndef H_OPTIONS_H
#define H_OPTIONS_H


#include <string>
#include <iostream>


using std::string;
using std::cout;
using std::endl;


int ArgPos(char *str, int argc, char **argv) {
	int a;
	for (a = 1; a < argc; a++) {
		if (!strcmp(str, argv[a])) {
			if (a == argc - 1) {
				std::cout << "Argument missing for " << str << endl;
				exit(1);
			}
			return a;
		}
	}
	return -1;
}


struct HOptions {

	HOptions() {
		vectorsFile = "";
		idsFile = "";
		outFile = "";
		vecDim = 0;
		numClusters = 0;
		numThreads = 1;
		maxVectors = -1;
		maxIters = 20;
		eps = 0.00001f;
		saStart = 0.2f;
		saIters = 0;
	}

	string vectorsFile;
	string idsFile;
	string outFile;
	int vecDim;
	int numClusters;
	int numThreads;
	int maxVectors;
	int maxIters;
	float eps;
	float saStart; // A probability parameter for simulated annealing
	float saIters; // The number of simulated annealing iterations

};



#endif


