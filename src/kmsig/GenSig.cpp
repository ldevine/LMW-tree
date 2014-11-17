
#include <random>
#include <iostream>
#include <fstream>
#include <thread>
#include <algorithm>

#include "SVector.h"
#include "VectorGenerator.h"
#include "HUtils.h"
#include "tinyformat.h"


int vecDimensions;
int seed;
int numVecs;
string vectorsFile;
float p; // This is parameter p for the Bernoulli distribution

void gen(vector<SVector<bool>*> &vecs, int dim, int rngSeed, int numVectors, float pParam) {

	std::mt19937 gen(rngSeed);
	std::bernoulli_distribution dis(pParam);

	auto genRnd = [&]() -> bool {
		return dis(gen);		
	};

	VectorGenerator<decltype(genRnd), SVector<bool>> vecGenerator;
	vecGenerator.genVectors(vecs, numVectors, genRnd, dim);
}


void writeFile(vector<SVector<bool>*> &vecs, int dim, string fileName ) {

	std::ofstream vecsOutStream(fileName, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);

	int bytesPerVector = dim / 8; // 8 bits / byte

	if (!vecsOutStream.is_open()) return;

	vecsOutStream << dim << endl;
	for (SVector<bool>* vec : vecs) {
		vecsOutStream.write((char*)vec->getData(), bytesPerVector);
		//cout << endl << ".";
	}
	
	vecsOutStream.close();

	//vecsOutStream..close();
}


void readFile(vector<SVector<bool>*> &vecs, string fileName) {

	std::ifstream vecsInStream(fileName, std::ifstream::binary);

	int dim = 0, vecInBytes;
	SVector<bool> *vec;
	string line;
	char *vecBuf;

	if (!vecsInStream.is_open()) return;

	// Get vector dimension
	getline(vecsInStream, line);	
	dim = std::stoi(line);
	vecInBytes = dim / 8;
	cout << endl << dim;

	// Allocate memory for buffer
	vecBuf = new char[vecInBytes];
	
	while (vecsInStream.read(vecBuf, vecInBytes)) {
		//cout << endl << vecsInStream.gcount() << endl;
		vec = new SVector<bool>(vecBuf, dim);
		vecs.push_back(vec);
	}	

	vecsInStream.close();

	cout << endl << vecs.size() << endl;
}

void parseOptions(int argc, char **argv) {

	int i;
	if ((i = ArgPos((char *)"-dim", argc, argv)) > 0) vecDimensions = atoi(argv[i + 1]);		
	if ((i = ArgPos((char *)"-seed", argc, argv)) > 0) seed = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-out", argc, argv)) > 0) vectorsFile = argv[i + 1];
	if ((i = ArgPos((char *)"-numvecs", argc, argv)) > 0) numVecs = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-p", argc, argv)) > 0) p = atof(argv[i + 1]);
}


int main(int argc, char** argv) {

	// Set default option values
	p = 0.5;
	seed = 12;
	vecDimensions = 128;
	numVecs = 100;
	vectorsFile = "out.dat";

	// Get command line options
	parseOptions(argc, argv);
	
	vector<SVector<bool>*> vecs;

	gen(vecs, vecDimensions, seed, numVecs, p);

	// Write file
	string fileName = tfm::format("%s.bin", vectorsFile);
	writeFile(vecs, vecDimensions, fileName);

	HUtils::purge(vecs);

	vecs.clear();

	//--------------------
	// Some testing ...
	//--------------------

	//readFile(vecs, vectorsFile);
	//cout << endl;
	//vecs[3]->print();
	// Clean up allocated vectors
	//HUtils::purge(vecs);

	return 0;
}



