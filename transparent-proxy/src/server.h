#pragma once

#include "concurrency/queue.h"
#include "concurrency/threadpool.h"

class ProxyServer {
    private:
        int sockfd_;

        // server thread pool for concurrency
        ThreadPool _thread_pool;

        // client file descriptor request queue
        Queue<int> *_requests_queue;

        void processRequests();

    public:
        void setListeningSocket(int);

        void createClientThreadPool(int);

        void start();
};