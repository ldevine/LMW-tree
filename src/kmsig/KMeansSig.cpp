
#include <chrono>
#include <iostream>
#include <fstream>

#include "SVector.h"
#include "KMeans.h"
#include "RandomSeeder.h"
#include "Optimizer.h"
#include "Prototype.h"
#include "Distance.h"

#include "tinyformat.h"


typedef SVector<bool> vecType;
typedef RandomSeeder<vecType> RandomSeeder_t;
typedef Optimizer<vecType, hammingDistance, Minimize, meanBitPrototype2> OPTIMIZER;
typedef KMeans<vecType, RandomSeeder_t, OPTIMIZER> KMeans_t;


string vectorsFile;
string idsFile;
string outFile;
int numClusters;
int numThreads;
int hasIds;


// Read vectors without an associated identifier file
void readVectors(vector<SVector<bool>*> &vectors, string signatureFile, size_t maxVectors) {
	
	using namespace std;

	cout << signatureFile << endl;

	size_t sigSize;
	string line;
	char *vecBuf;

	// setup stream
	ifstream sigStream(signatureFile, ios::in | ios::binary);

	if (!sigStream.is_open()) return;
	
	// Get vector dimension
	getline(sigStream, line);
	sigSize = std::stoi(line);
	const size_t numBytes = sigSize / 8;
	cout << endl << numBytes;
	
	// Allocate buffer for vector
	char *data = new char[numBytes];
	
	if (!sigStream) {
		cout << "unable to open file" << endl;
		return;
	}

	// id for each vector
	string id;

	// read data
	int count = 0; // set vector counter to 0. Use this for creating ids
	while (sigStream.read(vecBuf, numBytes)) {
		SVector<bool>* vector = new SVector<bool>(data, sigSize);

		id = tfm::format("%d", count);

		vector->setID(id);
		vectors.push_back(vector);
		
		if (vectors.size() % 1000 == 0) {
			cout << "." << flush;
		}
		if (vectors.size() % 100000 == 0) {
			cout << vectors.size() << flush;
		}
		if (maxVectors != -1 && vectors.size() == maxVectors) {
			break;
		}
	}
	cout << endl << vectors.size() << endl;
	delete[] data;
}


// Read vectors and associated identifier file

void readVectors(vector<SVector<bool>*> &vectors, string idFile, string signatureFile,
	size_t maxVectors) {
	using namespace std;

	cout << idFile << endl << signatureFile << endl;

	size_t sigSize;
	string line;
	char *vecBuf;

	// setup stream
	ifstream docidStream(idFile);
	ifstream sigStream(signatureFile, ios::in | ios::binary);

	if (!docidStream || !sigStream) {
		cout << "unable to open file" << endl;
		return;
	}

	// Get vector dimension
	getline(sigStream, line);
	sigSize = std::stoi(line);
	const size_t numBytes = sigSize / 8;
	cout << endl << numBytes;

	// Allocate buffer for vector
	char *data = new char[numBytes];

	if (!sigStream) {
		cout << "unable to open file" << endl;
		return;
	}

	string id;

	// read data
	while (getline(docidStream, id)) {
		sigStream.read(data, numBytes);
		SVector<bool>* vector = new SVector<bool>(data, sigSize);
		vector->setID(id);
		vectors.push_back(vector);
		if (vectors.size() % 1000 == 0) {
			cout << "." << flush;
		}
		if (vectors.size() % 100000 == 0) {
			cout << vectors.size() << flush;
		}
		if (maxVectors != -1 && vectors.size() == maxVectors) {
			break;
		}
	}
	cout << endl << vectors.size() << endl;
	delete[] data;
}


void sigKmeansCluster(vector<SVector<bool>*> &vectors, int numClusters, int numThreads) {
	
	int maxiters = 10;
	KMeans_t clusterer(numClusters, numThreads);
	clusterer.setMaxIters(maxiters);
		
	cout << "clustering " << vectors.size() << " vectors into " << numClusters
		<< " clusters using kmeans with maxiters = " << maxiters
		<< std::endl;
		
	auto start = std::chrono::steady_clock::now();

	// Cluster
	vector<Cluster<vecType>*>& clusters = clusterer.cluster(vectors);
	
	auto end = std::chrono::steady_clock::now();
	auto diff_msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	cout << "cluster count = " << clusters.size() << std::endl;
	cout << "RMSE = " << clusterer.getRMSE() << std::endl;

}


void parseOptions(int argc, char **argv) {
	int i;
	if ((i = ArgPos((char *)"-in", argc, argv)) > 0) vectorsFile = argv[i + 1];
	if ((i = ArgPos((char *)"-clusters", argc, argv)) > 0) numClusters = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-threads", argc, argv)) > 0) numThreads = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-out", argc, argv)) > 0) outFile = argv[i + 1];
	if ((i = ArgPos((char *)"-ids", argc, argv)) > 0) hasIds = atoi(argv[i + 1]);
}


int main(int argc, char** argv) {

	vectorsFile = "";
	idsFile = "";
	hasIds = 0;
	outFile = "";

	string tempStr;
	int maxVectors = -1;

	numClusters = 10;
	numThreads = 1;

	// Process options
	parseOptions(argc, argv);
	
	if (vectorsFile.length() == 0) {
		cout << endl << "Must provide an input file name.";
		return 0;
	}

	tempStr = vectorsFile;
	vectorsFile = tempStr + ".bin";
	if (hasIds) idsFile = tempStr + ".ids";
	
	vector<SVector<bool>*> vectors;

	// Load vectors
	if (hasIds) {
		readVectors(vectors, idsFile, vectorsFile, maxVectors);
	}
	else {
		readVectors(vectors, vectorsFile, maxVectors);
	}

	// Cluster
	sigKmeansCluster(vectors, numClusters, numThreads);

	return 0;
}





