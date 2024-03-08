#pragma once

#include <vector>
#include <thread>

#include "../logger/logger.h"

class ThreadPool {
    private:
        std::vector<std::thread> threads_;
        int num_threads_;
        const static int MAX_THREADS = 20;

    public:
        ThreadPool();

        void init(int);

        template <typename T>
        void createWorkerThreads(T && function) {
            Logger::debug("---------- Creating worker threads for the server -----------");

            for (int i=0; i<num_threads_; i++) {
                threads_.emplace_back(std::thread(std::forward<T>(function)));
            }
            
        }

        void joinWorkerThreads();
};