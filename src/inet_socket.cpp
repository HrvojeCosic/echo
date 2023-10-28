#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

#include "../include/socket.hpp"

namespace echoserver {

int InetSocket::createAndBind() {
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == -1) {
        throw std::runtime_error("Failed to create the listening socket");
    }

    int enable = 1;
    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
        throw std::runtime_error("Failed to set socket options");
    }

    struct sockaddr_in serverAddr {};

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(listenSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        throw std::runtime_error("Failed to bind the internet domain listening socket");
    }

    return listenSocket;
}

int InetSocket::setupNewConnection() {
    while (true) {
        struct sockaddr_in clientAddr {};

        socklen_t clientAddrLen = sizeof(clientAddr);

        int clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                return clientSocket; // No more incoming connections
            } else {
                throw std::runtime_error("Failed to accept a new internet socket connection");
            }
        }

        initOptions(clientSocket);

        char newClientIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), newClientIp, INET_ADDRSTRLEN);
        std::cout << "Accepted connection from " << newClientIp << ":" << ntohs(clientAddr.sin_port) << std::endl;

        return clientSocket;
    }
}

void InetSocket::initOptions(int socket) {
    if (fcntl(listenSocket, F_SETFL, O_NONBLOCK) == -1) {
        throw std::runtime_error("Failed to set the listening internet socket to non-blocking mode");
    }

    int flag = 1;
    if (setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)) == -1) {
        throw std::runtime_error("Failed to disable the Nagle algorithm");
    }
}

void InetSocket::destroy() { close(listenSocket); }
} // namespace echoserver