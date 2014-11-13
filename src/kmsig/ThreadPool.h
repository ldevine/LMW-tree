#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

//------------------------------------------------
// Simple Thread Pool
// See: http://progsch.net/wordpress/?p=81
// 
// 
// Some alterations by Lance.
// Changed the constructor.
// Added init(size_t numThreads) method.
//------------------------------------------------

class ThreadPool {
public:
    ThreadPool();
	void init(size_t numThreads);
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();
private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;
    // the task queue
    std::queue< std::function<void()> > tasks;
    
    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

void ThreadPool::init(size_t threads) {
	for (size_t i = 0; i<threads; ++i)
		workers.emplace_back(
		[this]
	{
		for (;;)
		{
			std::unique_lock<std::mutex> lock(this->queue_mutex);
			while (!this->stop && this->tasks.empty())
				this->condition.wait(lock);
			if (this->stop && this->tasks.empty())
				return;
			std::function<void()> task(this->tasks.front());
			this->tasks.pop();
			lock.unlock();
			task();
		}
	}
	);
}
// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool()
    :   stop(false)
{

}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    typedef typename std::result_of<F(Args...)>::type return_type;
    
    // don't allow enqueueing after stopping the pool
    if(stop)
        throw std::runtime_error("enqueue on stopped ThreadPool");

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push([task](){ (*task)(); });
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(size_t i = 0;i<workers.size();++i)
        workers[i].join();
}

#endif
