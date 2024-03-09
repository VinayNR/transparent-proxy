#pragma once

#include "concurrency/queue.h"
#include "concurrency/threadpool.h"
#include "http/request.h"
#include "http/response.h"

#include <vector>

class ProxyServer {
    private:
        int sockfd_;

        // server thread pool for concurrency
        ThreadPool _thread_pool;

        // client file descriptor request queue
        Queue<int> *_requests_queue;

        void processRequests();

        std::vector<char> readDataOnSocket(int);

        HttpRequest* readRequest(int);
        int writeRequest(int, HttpRequest *);
        HttpResponse * readResponse(int);
        int writeResponse(int, HttpResponse *);

    public:
        void setListeningSocket(int);

        void createClientThreadPool(int);

        void start();
};