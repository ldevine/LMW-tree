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
#include "HOptions.h"

#include "tinyformat.h"


typedef SVector<bool> vecType;
typedef RandomSeeder<vecType> RandomSeeder_t;
typedef DSquaredSeeder<vecType, hammingDistance> DSSeeder_t;
typedef Optimizer<vecType, hammingDistance, Minimize, meanBitPrototype2> OPTIMIZER;
typedef KMeans<vecType, RandomSeeder_t, OPTIMIZER> KMeans_t;
//typedef KMeans<vecType, DSSeeder_t, OPTIMIZER> KMeans_t;


// All our options
HOptions options;


// Read vectors without an associated identifier file
void readVectors(vector<SVector<bool>*> &vectors, HOptions &options) {
	//string signatureFile, size_t maxVectors, int dim = 0) {
	
	using namespace std;

	cout << endl << "Reading vectors from " << options.vectorsFile << endl;

	size_t sigSize;
	string line;

	// setup stream
	ifstream sigStream(options.vectorsFile, ios::in | ios::binary);

	if (!sigStream.is_open()) return;
	
	// Get vector dimension
	if (options.vecDim == 0) {
		getline(sigStream, line);
		options.vecDim = std::stoi(line);
	}
	sigSize = options.vecDim;

	const size_t numBytes = sigSize / 8;
	//cout << endl << numBytes;
	
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
		if (options.maxVectors == vectors.size()) {
			break;
		}

		count++;
	}
	cout << endl << vectors.size() << " vectors." << endl;
	delete[] data;
}


// Read vectors and associated identifier file
void readVectorsAndIDs(vector<SVector<bool>*> &vectors, HOptions &options) {
		
	using namespace std;

	cout << endl << "Reading vectors from " << options.vectorsFile << endl;
	cout << endl << "Reading ids from " << options.idsFile << endl;

	size_t sigSize;
	string line;

	// setup stream
	ifstream docidStream(options.idsFile);
	ifstream sigStream(options.vectorsFile, ios::in | ios::binary);

	if (!docidStream || !sigStream) {
		cout << "unable to open file" << endl;
		return;
	}

	// Get vector dimension
	if (options.vecDim == 0) {
		getline(sigStream, line);
		sigSize = std::stoi(line);
		options.vecDim = sigSize;
	}
	else sigSize = options.vecDim;

	const size_t numBytes = sigSize / 8;
	//cout << endl << numBytes;

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
		if (vectors.size() == options.maxVectors) {
			break;
		}
	}

	cout << endl << vectors.size() << " vectors." << endl;
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

	hammingDistance _distance;
	std::ofstream ofs(fileName);
	for (size_t i = 0; i < clusters.size(); ++i) {
		for (SVector<bool>* vector : clusters[i]->getNearestList()) {
			ofs << vector->getID() << " " << i << endl;
			// ofs << vector->getID() << " " << i << " " << _distance(vector, 
			//	clusters[i]->getCentroid()) << endl;
		}
	}
}


void sigKmeansCluster(vector<SVector<bool>*> &vectors, HOptions &options) {
			
	KMeans_t clusterer(options.numClusters, options.numThreads);
	clusterer.setMaxIters(options.maxIters);
	clusterer.setEps(options.eps);
	clusterer.setSAIters(options.saIters);
	clusterer.setSAStart(options.saStart);
			
	cout << endl << "Clustering ... " << endl; 

	cout << "number of threads = " << options.numThreads << endl;
	cout << "number of vectors = " << vectors.size() << endl;
	cout << "vector dimension = " << options.vecDim << endl;
	cout << "number of clusters = " << options.numClusters << endl;
	cout << "maxiters = " << options.maxIters << endl;
	cout << "eps = " << options.eps << endl;
	cout << "sim. annealing iters = " << options.saIters << endl;

	auto start = std::chrono::steady_clock::now();

	// Cluster
	vector<Cluster<vecType>*>& clusters = clusterer.cluster(vectors);

	auto end = std::chrono::steady_clock::now();
	auto diff_msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	vector<float> &rmses = clusterer.getRMSEs();
	writeRMSEs(rmses, "rmses3.txt");
	
	cout << endl << endl;
	cout << "cluster count = " << clusters.size() << std::endl;
	cout << "RMSE = " << clusterer.getRMSE() << std::endl;
	cout << "Time = " << ((float)diff_msec.count())/1000.0f << std::endl;

	if (options.outFile.length() > 0) writeClusters(clusters, options.outFile);
}


void parseOptions(int argc, char **argv) {
	int i;
	if ((i = ArgPos((char *)"-in", argc, argv)) > 0) options.vectorsFile = argv[i + 1];
	if ((i = ArgPos((char *)"-clusters", argc, argv)) > 0) options.numClusters = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-threads", argc, argv)) > 0) options.numThreads = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-out", argc, argv)) > 0) options.outFile = argv[i + 1];
	if ((i = ArgPos((char *)"-ids", argc, argv)) > 0) options.idsFile = argv[i + 1];
	if ((i = ArgPos((char *)"-maxvecs", argc, argv)) > 0) options.maxVectors = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-maxiters", argc, argv)) > 0) options.maxIters = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-dim", argc, argv)) > 0) options.vecDim = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-eps", argc, argv)) > 0) options.eps = atof(argv[i + 1]);
	if ((i = ArgPos((char *)"-sastart", argc, argv)) > 0) options.saStart = atof(argv[i + 1]);
	if ((i = ArgPos((char *)"-saiters", argc, argv)) > 0) options.saIters = atoi(argv[i + 1]);
}


int main(int argc, char** argv) {

	// Process options
	parseOptions(argc, argv);
	
	if (options.vectorsFile.length() == 0) {
		cout << endl << "Must provide an input file name.";
		return 0;
	}

	// Vectors to hold all the data
	vector<SVector<bool>*> vectors;

	// Load vectors
	if (options.idsFile.length()>0) {
		readVectorsAndIDs(vectors, options);
	}
	else {
		readVectors(vectors, options);
	}

	// Cluster
	sigKmeansCluster(vectors, options);

	return 0;
}





