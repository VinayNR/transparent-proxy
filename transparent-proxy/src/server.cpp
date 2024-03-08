#include "server.h"
#include "netutils/sockets.h"
#include "netutils/utils.h"
#include "logger/logger.h"

#include <sys/socket.h>
#include <iostream>
#include <getopt.h>

void ProxyServer::setListeningSocket(int port) {
    if ((sockfd_ = setupListeningSocket(port, SOCK_STREAM)) == -1) {
        std::cerr << "Error creating socket" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ProxyServer::createClientThreadPool(int num_threads) {
    // create a thread pool of worker threads for the Proxy server
    _thread_pool.init(num_threads);

    // initialize the client requests queue
    _requests_queue = new Queue<int>;

    // Create the lambda function for the server worker's task
    auto client_handling_task = [this]() {
        this->processRequests();
    };

    _thread_pool.createWorkerThreads(client_handling_task);
}


void ProxyServer::processRequests() {
    Logger::debug("------ Proxy Server Thread Pool: ", std::this_thread::get_id(), " ------- ");

    while (true) {
        // take a connection from the queue
        int client_sockfd = _requests_queue->dequeue();
        
        // if the client socket is valid
        if (client_sockfd != -1) {
            Logger::debug(std::this_thread::get_id(), " obtained a client socket: ", client_sockfd);

            // do something here
        }
    }
}

void ProxyServer::start() {
    // create a client thread pool
    createClientThreadPool(10);

    Logger::debug("-------- Started Transparent Proxy ---------");

    // store client address during incoming connect requests
    int client_sockfd;

    while (true) {
        client_sockfd = SocketOps::acceptOnSocket(sockfd_);

        // add the connection received to the queue
        _requests_queue->enqueue(client_sockfd);

        Logger::debug("Added client connection to queue: ", client_sockfd);
    }
}

void showUsage() {
    std::cout << "Usage: <Executable> <options>?" << std::endl;
    std::cout << "Options are explained below" << std::endl;
    std::cout << "-p <arg>: Specify the port number for the listening server. Ex: -p 8888" << std::endl;
    std::cout << "-l: Specify that the server should log requests" << std::endl;
    std::cout << "-h: Show the help section" << std::endl;
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        Logger::debug("Running the server with default parameters...");
    }

    int server_port = 6666;

    // parse command line options
    int option;
    const char* optstring = "p:lh";

    while ((option = getopt(argc, argv, optstring)) != -1) {
        switch (option) {
            case 'p':
                try {
                    server_port = std::stoi(optarg);
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid argument for port number: " << e.what() << "\n";
                    exit(EXIT_FAILURE);
                }
                break;
            case 'l':
                // set file logging
                Logger::setFileLogging();
                break;
            case 'h':
                showUsage();
                exit(EXIT_SUCCESS);
            case '?':
                std::cerr << "Unknown option or missing argument" << std::endl;
                exit(EXIT_FAILURE);
        }
    }

    // create an instance of the server
    ProxyServer server;

    // setup the server
    server.setListeningSocket(server_port);
    
    // start the server
    server.start();

    return 0;
}