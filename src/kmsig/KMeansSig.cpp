//---------------------------
//
// Author: Lance De Vine
//
// Based on parts from LMW-tree
// https://github.com/cmdevries/LMW-tree
//
//---------------------------

#include <chrono>
#include <iostream>
#include <fstream>

#include "SVector.h"
#include "KMeans.h"
#include "RandomSeeder.h"
#include "DSquaredSeeder.h"
#include "Optimizer.h"
#include "Prototype.h"
#include "Distance.h"

#include "tinyformat.h"


typedef SVector<bool> vecType;
typedef RandomSeeder<vecType> RandomSeeder_t;
typedef DSquaredSeeder<vecType, hammingDistance> DSSeeder_t;
typedef Optimizer<vecType, hammingDistance, Minimize, meanBitPrototype2> OPTIMIZER;
typedef KMeans<vecType, RandomSeeder_t, OPTIMIZER> KMeans_t;
//typedef KMeans<vecType, DSSeeder_t, OPTIMIZER> KMeans_t;


string vectorsFile;
string idsFile;
string outFile;
int vecDim;
int numClusters;
int numThreads;
int maxVectors;
int maxIters;
//int hasIds;
float eps;
// A probability parameter for simulated annealing
float sastart;
// The number of simulated annealing iterations
float saiters;


// Read vectors without an associated identifier file
void readVectors(vector<SVector<bool>*> &vectors, string signatureFile, size_t maxVectors, int dim = 0) {
	
	using namespace std;

	cout << signatureFile << endl;

	size_t sigSize;
	string line;

	// setup stream
	ifstream sigStream(signatureFile, ios::in | ios::binary);

	if (!sigStream.is_open()) return;
	
	// Get vector dimension
	if (dim == 0) {
		getline(sigStream, line);
		sigSize = std::stoi(line);
	}
	else sigSize = dim;

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
	while (sigStream.read(data, numBytes)) {
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

		count++;
	}
	cout << endl << vectors.size() << endl;
	delete[] data;
}


// Read vectors and associated identifier file

void readVectors(vector<SVector<bool>*> &vectors, string idFile, string signatureFile, size_t maxVectors, int dim = 0) {
	using namespace std;

	cout << idFile << endl << signatureFile << endl;

	size_t sigSize;
	string line;

	// setup stream
	ifstream docidStream(idFile);
	ifstream sigStream(signatureFile, ios::in | ios::binary);

	if (!docidStream || !sigStream) {
		cout << "unable to open file" << endl;
		return;
	}

	// Get vector dimension
	if (dim == 0) {
		getline(sigStream, line);
		sigSize = std::stoi(line);
	}
	else sigSize = dim;

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


void writeRMSEs(vector<float> &rmses, string fileName) {

	std::ofstream fout;
	fout.open(fileName);
	for (int i = 0; i < (rmses.size() - 1); i++) {
		fout << rmses[i] << endl;
	}
	fout << rmses[rmses.size() - 1] << endl;
	fout.close();
}

void writeClusters(vector<Cluster<vecType>*>& clusters, string fileName) {

	std::ofstream ofs(fileName);
	for (size_t i = 0; i < clusters.size(); ++i) {
		for (SVector<bool>* vector : clusters[i]->getNearestList()) {
			ofs << vector->getID() << " " << i << endl;
		}
	}
}


void sigKmeansCluster(vector<SVector<bool>*> &vectors, int numClusters, int numThreads, string clusterFile) {
	
	int maxiters = 30;
	KMeans_t clusterer(numClusters, numThreads);
	clusterer.setMaxIters(maxiters);
		
	cout << "clustering " << vectors.size() << " vectors into " << numClusters
		<< " clusters using kmeans with maxiters = " << maxiters
		<< std::endl;
		
	auto start = std::chrono::steady_clock::now();

	// Cluster
	vector<Cluster<vecType>*>& clusters = clusterer.cluster(vectors);
	
	vector<float> &rmses = clusterer.getRMSEs();
	writeRMSEs(rmses, "rmses3.txt");

	auto end = std::chrono::steady_clock::now();
	auto diff_msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	cout << "cluster count = " << clusters.size() << std::endl;
	cout << "RMSE = " << clusterer.getRMSE() << std::endl;
	cout << "Time = " << ((float)diff_msec.count())/1000.0f << std::endl;

	if (clusterFile.length() > 0) writeClusters(clusters, clusterFile);

}


void parseOptions(int argc, char **argv) {
	int i;
	if ((i = ArgPos((char *)"-in", argc, argv)) > 0) vectorsFile = argv[i + 1];
	if ((i = ArgPos((char *)"-clusters", argc, argv)) > 0) numClusters = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-threads", argc, argv)) > 0) numThreads = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-out", argc, argv)) > 0) outFile = argv[i + 1];
	if ((i = ArgPos((char *)"-ids", argc, argv)) > 0) idsFile = argv[i + 1];
	if ((i = ArgPos((char *)"-maxvecs", argc, argv)) > 0) maxVectors = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-maxiters", argc, argv)) > 0) maxIters = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-dim", argc, argv)) > 0) vecDim = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-eps", argc, argv)) > 0) eps = atof(argv[i + 1]);
	if ((i = ArgPos((char *)"-sastart", argc, argv)) > 0) sastart = atof(argv[i + 1]);
	if ((i = ArgPos((char *)"-saiters", argc, argv)) > 0) saiters = atoi(argv[i + 1]);
}


int main(int argc, char** argv) {

	// Default parameter values
	vectorsFile = "";
	idsFile = "";
	outFile = "";
	maxVectors = -1;
	vecDim = 0;
	numClusters = 10;
	numThreads = 1;
	maxIters = 20;
	eps = 0;
	sastart = 0.0f;
	saiters = 0;

	// Process options
	parseOptions(argc, argv);
	
	if (vectorsFile.length() == 0) {
		cout << endl << "Must provide an input file name.";
		return 0;
	}

	vector<SVector<bool>*> vectors;

	// Load vectors
	if (idsFile.length()>0) {
		readVectors(vectors, idsFile, vectorsFile, maxVectors, vecDim);
	}
	else {
		readVectors(vectors, vectorsFile, maxVectors, vecDim);
	}

	// Cluster
	sigKmeansCluster(vectors, numClusters, numThreads, outFile);

	return 0;
}





