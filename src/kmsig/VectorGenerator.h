#ifndef VECTOR_GENERATOR_H
#define VECTOR_GENERATOR_H


#include "SVector.h"


template <typename RndGen, typename Vec> 
class VectorGenerator {

public:

	VectorGenerator() {};
	virtual ~VectorGenerator() {};

	static void fillVector(Vec *v, RndGen &rndGen) {	
		for (auto it = v->begin(); it != v->end(); it++) {
			*it = rndGen();
		}		
	}

	// Fills one vector with random values
	static Vec* genVector(RndGen &rndGen, size_t size) {
		Vec* v = new Vec(size);
		fillVector( v, rndGen);
		return v;
	}
		
	static void genVectors(vector<Vec*> &vecs, int numVecs, RndGen &rndGen, size_t size) {
		for (int i=0; i<numVecs; i++) {
			vecs.push_back( new Vec(size) );
		}
		fillVectors( vecs, numVecs, rndGen);
	}

	static void fillVectors(vector<Vec*> &vecs, int numVecs, RndGen &rndGen) {
		for (Vec* v : vecs) {
			fillVector(v, rndGen);
		}
	}
	
};


template <typename RndGen>
class VectorGenerator<RndGen, SVector<bool> > {

public:

	VectorGenerator() {};
	virtual ~VectorGenerator() {};

	// Fills one vector with random values
	static SVector<bool>* genVector(RndGen &rndGen, size_t size) {
		SVector<bool>* v = new SVector<bool>(size);
		fillVector( v, rndGen);
		return v;
	}
		
	static void genVectors(vector<SVector<bool>*> &vecs, int numVecs, RndGen &rndGen, size_t size) {
		for (int i=0; i<numVecs; i++) {
			vecs.push_back( new SVector<bool>(size) );
		}
		fillVectors(vecs, numVecs, rndGen);
	}

	static void fillVector(SVector<bool> *v, RndGen &rndGen) {
		int value;
		for (int i=0; i<v->size(); i++) {
			value = rndGen();
			if (value) v->set(i);
		}
	}

	static void fillVectors(vector<SVector<bool>*> &vecs, int numVecs, RndGen &rndGen) {
		for (SVector<bool>* v : vecs) {
			fillVector(v, rndGen);
		}
	}
	
};


#endif



