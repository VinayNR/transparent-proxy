#include "server.h"
#include "net/sockets.h"
#include "net/utils.h"
#include "utils/utils.h"
#include "logger/logger.h"
#include "http/request.h"
#include "http/response.h"

#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#ifdef __linux__
#include <netinet/ip.h>
#include <linux/netfilter_ipv4.h> // For SO_ORIGINAL_DST
#define ORIGINAL_DST_OPTION SO_ORIGINAL_DST
#define ORIGINAL_DST_LEVEL SOL_IP
#elif defined(__APPLE__)
#include <netinet/ip.h>
#define ORIGINAL_DST_OPTION 1
#define ORIGINAL_DST_LEVEL IPPROTO_IP
#endif

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

std::vector<char> ProxyServer::readRequest() {
    std::cout << std::endl << " ----------- Inside Read data ----------- " <<std::endl;

    int buffer_size = 0;
    char *buffer = nullptr;
    std::vector<char> v;

    std::size_t content_length = 0, total_length = 0, header_size = 0;
    
    do {
        if ((buffer_size = SocketOps::receive(sockfd_, buffer, 4096, 10)) <= 0) {
            std::cout << " ----------- The buffer size is less than 1, breaking ----------- " << std::endl;
            break;
        }

        // find out the content length of the request
        if (content_length == 0) {
            // get the content length before deserialization of the data
            content_length = searchKeyValueFromRawString(buffer, "Content-Length: ", '\r');
            std::cout << " ----- Found content length ------ " << std::endl;
            std::cout << content_length << std::endl;

            // get the total number of bytes until the start of body
            const char* doubleCRLF = strstr(buffer, "\r\n\r\n");
            if (doubleCRLF != nullptr) {
                header_size = doubleCRLF - buffer;
                std::cout << "Header Size: " << header_size << std::endl;
            }

            // check for content_length_pos
            if (content_length == 0) {
                // the request had no content length header, so breaking out
                v.insert(v.end(), buffer, buffer + buffer_size);
                total_length = buffer_size;
                break;
            }
        }

        // copy buffer into data
        v.insert(v.end(), buffer, buffer + buffer_size);
        total_length += buffer_size;

        // check if everything is read from the socket
        if (content_length + header_size <= total_length) {
            break;
        }

        // clear the buffer
        deleteAndNullifyPointer(buffer, true);
    } while (buffer_size > 0);

    // clear the buffer
    deleteAndNullifyPointer(buffer, true);

    if (total_length > 0) {
        std::cout << "Length of data read: " << total_length << std::endl;
        std::cout << " -------- Finished Read Data ------- " << std::endl;
    }

    return v;
}

void ProxyServer::processRequests() {
    Logger::debug("------ Proxy Server Thread Pool: ", std::this_thread::get_id(), " ------- ");

    HttpRequest *http_request;

    while (true) {
        // take a connection from the queue
        int client_sockfd = _requests_queue->dequeue();
        
        // if the client socket is valid
        if (client_sockfd != -1) {
            Logger::debug(std::this_thread::get_id(), " obtained a client socket: ", client_sockfd);

            // read an entire request here
            std::vector<char> vec_data = readRequest();

            // convert the request to an object
            if (vec_data.size() != 0) {
                http_request = new HttpRequest;
                http_request->setSerializedRequest(vec_data.data(), vec_data.size());
            }

            // deserialize the data into a http request object
            if (http_request->deserialize() == -1) {
                std::cerr << "Failed to deserialize request object" << std::endl;
                delete http_request;
                http_request = nullptr;
            }

            // send this request to server

            // Retrieve the original destination address and port
            struct sockaddr_in original_dst_addr;
            socklen_t original_dst_addr_len = sizeof(original_dst_addr);
            if (getsockopt(client_sockfd, ORIGINAL_DST_LEVEL, ORIGINAL_DST_OPTION, &original_dst_addr, &original_dst_addr_len) < 0) {
                perror("getsockopt");
                close(client_sockfd);
            }

            // Convert the original destination address to a string
            char original_dst_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &original_dst_addr.sin_addr, original_dst_str, sizeof(original_dst_str));

            // Print the original destination address and port
            Logger::debug("Original destination address: ", original_dst_str);
            Logger::debug("Original destination port: ", ntohs(original_dst_addr.sin_port));
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