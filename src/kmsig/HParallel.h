#ifndef H_PARALLEL
#define H_PARALLEL

#include "ThreadPool.h"

// Parallel for
template<typename Func>
void parallel_for(ThreadPool &pool, int first, int last, int grainSize, Func &f) {

	std::vector< std::future<void> > results;
	for (int it = first; it < last; it += grainSize) {
		auto func = [=]() {
			for (int i = it; i < last && i < (it + grainSize); i++) {
				f(i);
			}
		};
		results.push_back(pool.enqueue(func));
	}
	// Execute
	for (size_t i = 0; i<results.size(); ++i) results[i].get();
}


#endif




