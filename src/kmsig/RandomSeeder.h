#ifndef RANDOM_SEEDER_H
#define RANDOM_SEEDER_H

#include <algorithm>
#include <random>

#include "Seeder.h"


template <typename T>
class RandomSeeder : public Seeder<T> {

private:


public:

	RandomSeeder() {
	
	}

	// Pre: The centroids vector is empty
	void seed(vector<T*> &data, vector<T*> &centroids, int numCentres) {

		//cout << endl << numCentres;

		centroids.clear();

		vector<int> indices;
		indices.reserve(data.size());
		for (int i=0; i<data.size(); i++) indices.push_back(i);

		std::mt19937 gen{ std::random_device{}() };

		std::shuffle ( indices.begin(), indices.end(), gen );

		T *vec;

		for (int i=0; i<numCentres && i<data.size(); i++) {
			vec = new T(*data[indices[i]]);
			centroids.push_back(vec); 
		}

		//std::cout << ".";
	}
	
};


#endif
