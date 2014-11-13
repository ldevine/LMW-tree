#ifndef D_SQUARED_SEEDER_H
#define D_SQUARED_SEEDER_H

#include <random>

#include "Seeder.h"
#include "StdIncludes.h"

namespace lmw {

template <typename T, typename DistanceFunc>
class DSquaredSeeder : public Seeder<T> {

private:

	DistanceFunc df;
	int numCentres;

	std::default_random_engine gen;
	std::uniform_real_distribution<float> uniDist{ 0, 1 };


public:

	DSquaredSeeder() {
		gen.seed((unsigned int)std::time(0));
	}

	// Pre: The centroids vector is empty
	void seed(vector<T*> &data, vector<T*> &centroids, int numCentres) {

		this->numCentres = numCentres;
		int retries = 2 + (int) std::log(numCentres);

		chooseSmartCenters(data, centroids, numCentres, retries);
	}

    void chooseSmartCenters(vector<T*> &data, vector<T*> &centroids, int numCenters, int numLocalTries) {
		
        if (numLocalTries < 4) numLocalTries = 4;

		// temporary hack
		numLocalTries = 1;

        int i;
		float dist;
		int dataCount = data.size();
        float currentPot = 0;
		vector<float> closestDistSq;
		closestDistSq.reserve(dataCount);

		centroids.clear();

        // Choose one random center and set the closestDistSq values
		int index = (int)(uniDist(gen) * dataCount);
        centroids.push_back(new T(*data[index]));
        for (i = 0; i < dataCount; i++) {
            
			closestDistSq[i] = df(data[i], data[index]);
            
			currentPot += closestDistSq[i];
        }

        // Choose each center
        for (int centerCount = 1; centerCount < numCenters; centerCount++) {
            // Repeat several trials
            float bestNewPot = -1;
            int bestNewIndex = 0;
            for (int localTrial = 0; localTrial < numLocalTries; localTrial++) {

                // Choose our center - have to be slightly careful to return a valid answer even accounting
                // for possible rounding errors
				float randVal = uniDist(gen) * currentPot;
                for (index = 0; index < dataCount - 1; index++) {
                    if (randVal <= closestDistSq[index]) {
                        break;
                    } else {
                        randVal -= closestDistSq[index];
                    }
                }

                // Compute the new potential
                float newPot = 0;
                for (i = 0; i < dataCount; i++) {
                    dist = df(data[i], data[index]);
                    newPot += std::min(dist, closestDistSq[i]);
                }

                // Store the best result
                if (bestNewPot < 0 || newPot < bestNewPot) {
                    bestNewPot = newPot;
                    bestNewIndex = index;
                }
            }

            // Add the appropriate center
            centroids.push_back(new T(*data[bestNewIndex]));
            currentPot = bestNewPot;
            for (i = 0; i < dataCount; i++) {
                dist = df(data[i], data[bestNewIndex]);
                closestDistSq[i] = std::min(dist, closestDistSq[i]);
            }
        }
		
    }

	
};

} // namespace lmw

#endif



