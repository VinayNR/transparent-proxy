#include "utils.h"
#include "sockets.h"
#include "../logger/logger.h"

#include <cstring>  // for memset
#include <sys/socket.h>  // for SOCK_STREAM
#include <netdb.h>  // for addrinfo
#include <unistd.h> // for close()

int getServerInfo(const char *domain, const char *port, struct addrinfo *& serverinfo) {
    // set the hints
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // IPV4
    hints.ai_socktype = SOCK_STREAM;

    // get server address info structures
    if (getaddrinfo(domain, port, &hints, &serverinfo) != 0) {
        Logger::error("Error with get addr info");
        serverinfo = nullptr;
        return -1;
    }
    return 0;
}

void getHostAndPort(const char *fullDomain, char *& host, char *& port) {
    // Find the position of the colon (:) to separate host and port
    const char *colonPos = std::strrchr(fullDomain, ':');

    if (colonPos != nullptr) {
        // Calculate the length of the host and port
        std::size_t hostLength = colonPos - fullDomain;
        std::size_t portLength = std::strlen(colonPos + 1);

        // Allocate memory for host and port
        host = new char[hostLength + 1];
        port = new char[portLength + 1];

        // Copy host and port strings
        std::strncpy(host, fullDomain, hostLength);
        host[hostLength] = '\0';

        std::strcpy(port, colonPos + 1);
    } else {
        // No port found, set port to nullptr
        host = new char[std::strlen(fullDomain) + 1];
        std::strcpy(host, fullDomain);
        port = new char[3];
        memset(port, 0, 3);
        memcpy(port, "80", 2);
    }
}

int setupListeningSocket(int port, int socket_type) {
    Logger::debug(" ---------- Setting up listening socket ---------- ");

    // create a tcp socket
    int listen_socket_fd = SocketOps::createSocket(socket_type);

    if (listen_socket_fd == -1) {
        Logger::error("Failed to create a socket");
        return -1;
    }

    // Set SO_REUSEPORT option
    int optval = 1;
    if (setsockopt(listen_socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
        std::cerr << "Failed to set SO_REUSEPORT option" << std::endl;
        close(listen_socket_fd);
        return -1;
    }

    // display the socket
    Logger::debug("Socket FD: ", listen_socket_fd, " and port number: ", port);

    /*
    * setsockopt: Handy debugging trick that lets us rerun the server immediately after we kill it; 
    * otherwise we have to wait about 20 secs. 
    */
    setsockopt(listen_socket_fd, SOL_SOCKET, optval, static_cast<const void*>(&optval) , sizeof(int));

    // bind the socket to the port
    if (SocketOps::bindSocket(listen_socket_fd, std::to_string(port).c_str()) == -1) {
        Logger::error("Failed to bind socket");
        SocketOps::closeSocket(listen_socket_fd);
        return -1;
    }

    // listen on the socket for incoming connections
    if (socket_type == SOCK_STREAM && SocketOps::listenOnSocket(listen_socket_fd, 20) == -1) {
        Logger::error("Failed to listen on socket");
        SocketOps::closeSocket(listen_socket_fd);
        return -1;
    }

    Logger::debug(" ---------- Setting up listening socket complete ---------- ");

    return listen_socket_fd;
}

int connectToServer(const char *host, const char *port) {
    Logger::debug(" ---------- Connecting to Server ---------- ");

    // create a new TCP socket to open a connection to the server
    int client_socket_fd = SocketOps::createSocket(SOCK_STREAM);
    if (client_socket_fd == -1) {
        Logger::error("Failed to create a socket");
        return -1;
    }

    struct addrinfo *server_info = nullptr;
    getServerInfo(host, port, server_info);

    // check if the DNS lookup failed
    if (server_info == nullptr) {
        Logger::error("Failed to obtain IP of the server");
        close(client_socket_fd);
        return -1;
    }

    // connect to the remote server
    if (SocketOps::connectSocket(client_socket_fd, server_info) == -1) {
        Logger::error("Failed to connect to server");
        close(client_socket_fd);
        return -1;
    }

    return client_socket_fd;
}