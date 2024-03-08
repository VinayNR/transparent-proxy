#include "threadpool.h"

/*
* Source implementation
*/
ThreadPool::ThreadPool() {
    num_threads_ = MAX_THREADS;
}

void ThreadPool::init(int num_threads) {
    num_threads_ = num_threads > MAX_THREADS? MAX_THREADS: num_threads;
}

void ThreadPool::joinWorkerThreads() {
    for (int i=0; i<threads_.size(); i++) {
        threads_.at(i).join();
    }
}