#include "sockets.h"
#include "utils.h"
#include "../logger/logger.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// default constructor
SocketOps::SocketOps() {}

// network socket operations
int SocketOps::createSocket(int socket_type) {
    return socket(AF_INET, socket_type, 0);
}

void SocketOps::closeSocket(int sockfd) {
    close(sockfd);
}

int SocketOps::connectSocket(int sockfd, const struct addrinfo *server_info) {
    // connect to the remote server
    return connect(sockfd, server_info->ai_addr, server_info->ai_addrlen);
}

int SocketOps::bindSocket(int sockfd, const char *port) {
    struct addrinfo *local_address = nullptr;
    if (getServerInfo(NULL, port, local_address) == -1) {
        Logger::error("Failed to get address of local machine");
        return -1;
    }
    return bind(sockfd, local_address->ai_addr, local_address->ai_addrlen);
}

int SocketOps::listenOnSocket(int sockfd, int backlog) {
    return listen(sockfd, backlog);
}

int SocketOps::acceptOnSocket(int sockfd) {
    // store client address during incoming connect requests
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(client_addr);

    int new_sock_fd = accept(sockfd, (struct sockaddr *) &client_addr, &addr_size);

    Logger::debug("Accepted a request from Client IP: ", inet_ntoa(client_addr.sin_addr));

    return new_sock_fd;
}

int SocketOps::send(int sockfd, const char *buffer, size_t buffer_length) {
    ssize_t bytes_sent, total_sent = 0;
    while (total_sent < buffer_length) {
        bytes_sent = write(sockfd, buffer + total_sent, buffer_length - total_sent);
        if (bytes_sent <= 0) {
            perror("closed");
            return -1;
        }
        total_sent += bytes_sent;
    }
    Logger::info("Sent Bytes: ", total_sent);
    return total_sent;
}

int SocketOps::waitOnSocket(int sockfd, int timeout_sec) {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sockfd, &readSet);

    struct timeval timeout;
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;

    return select(sockfd + 1, &readSet, nullptr, nullptr, &timeout);
}

int SocketOps::receive(int sockfd, char *& buffer, int MAX_BUFFER_LENGTH, int timeout_sec) {
    int bytes_read = -1;

    buffer = new char[MAX_BUFFER_LENGTH];
    memset(buffer, 0, MAX_BUFFER_LENGTH);

    // wait on the socket to check if data is available on the socket
    int status = SocketOps::waitOnSocket(sockfd, timeout_sec);

    if (status <= 0) {
        Logger::debug("Socket had no data in the timeout interval");
        return status;
    }

    // read from the socket
    bytes_read = read(sockfd, buffer, MAX_BUFFER_LENGTH);
    Logger::info("Receive done with bytes read: ", bytes_read);
    
    return bytes_read;
}

int SocketOps::sendTo(int sockfd, const char *buffer, size_t size, struct sockaddr *remote_address) {
    socklen_t serverlen = sizeof(struct sockaddr);

    // Send the data
    int bytesSent = sendto(sockfd, buffer, size, 0, remote_address, serverlen);

    if (bytesSent == -1) {
        Logger::error("Error sending data");
        return -1;
    }

    return bytesSent;
}

int SocketOps::receiveFrom(int sockfd, char *& buffer, size_t MAX_BUFFER_LENGTH, struct sockaddr *& remote_address) {
    socklen_t serverlen = sizeof(struct sockaddr);

    buffer = new char[MAX_BUFFER_LENGTH];
    memset(buffer, 0, MAX_BUFFER_LENGTH);
    
    // Receive a packet
    int bytes_read = recvfrom(sockfd, buffer, MAX_BUFFER_LENGTH, 0, remote_address, &serverlen);

    if (bytes_read == -1)  {
        Logger::error("Received nothing on the socket");
        return -1;
    }

    Logger::info("Receive done with bytes read: ", bytes_read);

    return bytes_read;
}