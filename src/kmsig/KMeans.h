#ifndef KMEANS_H
#define KMEANS_H

#include <random>
#include <algorithm>

#include "Cluster.h"
#include "Clusterer.h"
#include "Seeder.h"
#include "Distance.h"


#include "HUtils.h"
#include "HParallel.h"



template <typename T, typename SEEDER, typename OPTIMIZER>
class KMeans : public Clusterer<T> {
public:

    KMeans(int numClusters) : 
		_numClusters(numClusters), 
		_seeder(new SEEDER()),
		_numThreads( 1 ) 
	{
		tPool.init(_numThreads);
    }

	KMeans(int numClusters, int numThreads) :
		_numClusters(numClusters),
		_seeder(new SEEDER()),
		_numThreads(numThreads)
	{
		tPool.init(_numThreads);		
	}

    ~KMeans() {
        // Need to clean up any created cluster objects
        HUtils::purge(_clusters);
    }

    void setNumClusters(size_t numClusters) {
        _numClusters = numClusters;
    }

    void setMaxIters(int maxIters) {
        _maxIters = maxIters;
    }

	void setEps(float eps) {
		_eps = eps;
	}

	void setSAStart(float sastart) {
		_saStart = sastart;
	}

	void setSAIters(int saIters) {
		_saIters = saIters;
	}

    void setEnforceNumClusters(bool enforceNumClusters) {
        _enforceNumClusters = enforceNumClusters;
    }

    int numClusters() {
        return _numClusters;
    }

	vector<float>& getRMSEs() {
		return rmses;
	}

    vector<Cluster<T>*>& cluster(vector<T*> &data) {

		if (_saIters > 0) {
			std::uniform_int_distribution<uint32_t>::param_type pt{ 0, (unsigned int)_numClusters - 1};
			uint_dist.param(pt);
		}
	
        HUtils::purge(_clusters);
        _clusters.clear();
        _finalClusters.clear();
        cluster(data, _numClusters);
        finalizeClusters(data);
        return _finalClusters;
    }
    
    /**
     * pre: cluster() has been called
     */
    double getRMSE() {
        double SSE = 0;
        size_t objects = 0;
        for (Cluster<T>* cluster : _clusters) {
            auto neighbours = cluster->getNearestList();
            objects += neighbours.size();
            SSE += _optimizer.sumSquaredError(cluster->getCentroid(), neighbours);
        }
        return sqrt(SSE / objects);
    }

private:
    void finalizeClusters(vector<T*> &data) {      
        // Create list of final clusters to return;
        bool emptyCluster = assignClusters(data);
        if (emptyCluster && _enforceNumClusters) {
            // randomly shuffle if k cluster were not created to enforce the number of clusters if required
            //std::cout << std::endl << "k-means is splitting randomly";
            vector<T*> shuffled(data);
            std::random_shuffle(shuffled.begin(), shuffled.end());
            {
                size_t step = shuffled.size(), i = 0, clusterIndex = 0;
                for (; i < shuffled.size(); i += step, ++clusterIndex) {
                    for (size_t j = i; j < i + step && j < shuffled.size(); ++j) {
                        _nearestCentroid[j] = clusterIndex;
                    }
                }
            }
            vectorsToNearestCentroid(data);
            recalculateCentroids(data);
            assignClusters(data);
        }
    }
    
    bool assignClusters(vector<T*> &data) {      
        // Create list of final clusters to return;
        bool emptyCluster = false;
        for (Cluster<T>* c : _clusters) {
            if (!c->getNearestList().empty()) {
                _finalClusters.push_back(c);
            } else {
                emptyCluster = true;
            }
        }
        return emptyCluster;
    }   

    /**
     * @param vectors       the vectors to form clusters for
     * @param clusters      the number of clusters to find (i.e. k)
     */
    void cluster(vector<T*> &data, size_t clusters) {
        // Setup initial state.
        _iterCount = 0;
        _numClusters = clusters;
        _nearestCentroid.resize(data.size());
        _seeder->seed(data, _centroids, _numClusters);

		float rmseOld, rmseCurr;

        // Create as many cluster objects as there are centroids
        for (T* c : _centroids) {
            _clusters.push_back(new Cluster<T>(c));
        }

        // First iteration
		cout << endl << "First iteration ...";

        vectorsToNearestCentroid(data);
		assignCentroids(data);
		
        if (_maxIters == 0) {
            return;
        }
        recalculateCentroids(data);

		rmseCurr = getRMSE();
		rmseOld = rmseCurr;
		rmses.push_back(rmseCurr);

		cout << endl << 1 << "  " << rmseCurr;

        if (_maxIters == 1) {
            return;
        }

        _converged = false;
        _iterCount = 1;
		//_saIterCount = 0;

		// Do standard k-means
		innerLoop(data);

		// Do annealing
		if (_saIters>0) {

			cout << endl << endl << "Commencing simulated annealing ... ";
						
			// Set up annealing schedule
			float aRate = _saStart;
			float saEnd = _saStart - (2.0f / 3.0f * _saStart);
			float saStep = (_saStart - saEnd) / _saIters;

			cout << endl << "Start = " << _saStart;
			cout << endl << "Steps = " << _saIters;
			cout << endl << "Decrement = " << saStep;

			for (int i = 0; i < _saIters; i++) {

				cout << endl << endl << "Annealing iteration " << i + 1 << endl;

				std::bernoulli_distribution::param_type pt{aRate-(i*saStep)};
				bern_dist.param(pt);

				// Do perturbing
				vectorsToNearestCentroid(data);
				assignPerturbCentroids(data);
				recalculateCentroids(data);

				_iterCount++;

				rmseCurr = getRMSE();
				rmses.push_back(rmseCurr);
				cout << endl << _iterCount << "  " << rmseCurr;

				// Do k-means iterations
				innerLoop(data);
			}	
		}

    }

	void innerLoop(vector<T*> &data)  {
		
		float rmseOld, rmseCurr;

		rmseCurr = getRMSE();
		rmseOld = rmseCurr;
		
		int innerIterCount = 0;

		_converged = false;

		while (!_converged) {
			
			vectorsToNearestCentroid(data);
			assignCentroids(data);
			recalculateCentroids(data);

			_iterCount++;
			innerIterCount++;

			rmseCurr = getRMSE();
			rmses.push_back(rmseCurr);
			cout << endl << _iterCount << "  " << getRMSE();

			if ((rmseOld - rmseCurr) < _eps) break;
			rmseOld = rmseCurr;

			if (innerIterCount >= _maxIters && innerIterCount >=0) {
				break;
			}			
		}
	}

	
	void assignPerturbCentroids(vector<T*> &data) {

		uint32_t idx;

		// Serial
		// Clear the nearest vectors in each cluster
		for (Cluster<T> *c : _clusters) {
			c->clearNearest();
		}

		// Accumlate into clusters
		for (size_t i = 0; i < data.size(); i++) {

			size_t nearest = _nearestCentroid[i];
			
			if (bern_dist(rng)) {
				nearest = uint_dist(rng);
				_nearestCentroid[i] = nearest;
			}
			
			_clusters[nearest]->addNearest(data[i]);			
		}
	}


	void assignCentroids(vector<T*> &data) {

		// Serial
		// Clear the nearest vectors in each cluster
		for (Cluster<T> *c : _clusters) {
			c->clearNearest();
		}

		// Accumlate into clusters
		for (size_t i = 0; i < data.size(); i++) {
			size_t nearest = _nearestCentroid[i];
			_clusters[nearest]->addNearest(data[i]);
			//cout << endl << nearest;
		}
	}


    /**
     * Assign vectors to nearest centroid.
     * Pre: seedCentroids() OR recalculateCentroids() has been called
     * Post: nearestCentroid contains all the indexes into centroids for the
     *       nearest centroid for the vector (nearestCentroid and vectors are
     *       aligned).
     * @return boolean indicating if there were any changes, i.e. was there
     *                 convergence
     */
    void vectorsToNearestCentroid(vector<T*> &data) {
        // Clear the nearest vectors in each cluster
        for (Cluster<T> *c : _clusters) {
            c->clearNearest();
        }
        _converged = true;

		
		//ThreadPool tPool(4);

		auto func = [=](int i) {
			auto nearest = _optimizer.nearest(data[i], _centroids);
			if (nearest.index != _nearestCentroid[i]) {
				_converged = false;
			}
			_nearestCentroid[i] = nearest.index;
		};

		parallel_for(tPool, 0, data.size(), 200, func);

		// make sure all memory writes are visible on all CPUs
		atomic_thread_fence(std::memory_order_release);
		
		/*
		for (size_t i = 0; i != data.size(); ++i) {
			//size_t nearest = nearestObj(data[i], _centroids);
			auto nearest = _optimizer.nearest(data[i], _centroids);
			//cout << endl << nearest.index;
			if (nearest.index != _nearestCentroid[i]) {
				_converged = false;
			}
			_nearestCentroid[i] = nearest.index;
		}*/		
    }

    /**
     * Recalculate centroids after vectors have been moved to there nearest
     * centroid.
     * Pre: vectorsToNearestCentroid() has been called
     * Post: centroids has been updated with new vector data
     */
    void recalculateCentroids(vector<T*> &data) {

		
		//ThreadPool tPool(4);

		
		auto func = [=](int i) { 
			Cluster<T>* c = _clusters[i];
			if (c->size() > 0) {
				_optimizer.updatePrototype(c->getCentroid(), c->getNearestList(), _weights);
			}
			else {
				cout << endl << "EMPTY CLUSTER !!!" << endl;
			}
		};

		parallel_for(tPool, 0, _clusters.size(), 10, func);

		//atomic_thread_fence(std::memory_order_release);

		/*
		for (size_t i = 0; i != _clusters.size(); ++i) {
			Cluster<T>* c = _clusters[i];
			if (c->size() > 0) {
				_optimizer.updatePrototype(c->getCentroid(), c->getNearestList(), _weights);
			}
		}*/
		
    }

	void print() {

		hammingDistance _distance;
		cout << endl << endl;

		// Iterate over cluster2

		for (int i = 0; i < 5; i++)  {
			Cluster<T> *c = _clusters[i];
			T *cent = c->getCentroid();
			cent->print();

			vector<T*>& members = c->getNearestList();

			for (T *v : members) {
				cout << endl;
				v->print();
				cout << "  " << _distance(cent, v);
			}

			cout << endl << endl;
		}
	}

	// For simulated annealing
	std::mt19937 rng;
	std::uniform_int_distribution<uint32_t> uint_dist;
	std::bernoulli_distribution bern_dist;

    SEEDER *_seeder;
    OPTIMIZER _optimizer;
    
    // enforce the number of clusters required
    // if less than k clusters are produced, shuffle vectors randomly and split into k cluster
    bool _enforceNumClusters = false;

    // present number of iterations
    int _iterCount = 0;

    // maximum number of iterations
    // -1 - run until complete convergence
    // 0 - only assign nearest neighbors after seeding
    // >= 1 - perform this many iterations
    int _maxIters = 100;

    // How many clusters should be found? i.e. k
    int _numClusters = 0;

    vector<T*> _centroids;
    vector<Cluster<T>*> _clusters;
    vector<Cluster<T>*> _finalClusters;

    // The centroid index for each vector. Aligned with vectors member variable.
    vector<size_t> _nearestCentroid;

    // Weights for prototype function (we don't have to use these)
    vector<int> _weights;

    // Residual for convergence
    float _eps = 0.00001f;

	// The starting annealing probability parameter
	float _saStart = 0.2f;

	// The number of annealing operations
	int _saIters = 0;

    // has the clustering converged
    std::atomic<bool> _converged; 

	// the number of threads
	int  _numThreads;

	ThreadPool tPool;

	vector<float> rmses;
};


#endif	/* KMEANS_H */



