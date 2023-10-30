#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

#include "../include/socket.hpp"

namespace echoserverclient {

int InetSocket::createAndBind() {
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd == INVALID_SOCKET_FD) {
        throw std::runtime_error("Failed to create the listening socket");
    }

    int enable = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
        throw std::runtime_error("Failed to set socket options");
    }

    struct sockaddr_in serverAddr {};

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(socketFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        throw std::runtime_error("Failed to bind the internet domain listening socket");
    }

    return socketFd;
}

int InetSocket::setupNewConnection() {
    while (true) {
        struct sockaddr_in clientAddr {};

        socklen_t clientAddrLen = sizeof(clientAddr);

        int clientSocket = accept(socketFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET_FD) {
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
    if (fcntl(socketFd, F_SETFL, O_NONBLOCK) == -1) {
        throw std::runtime_error("Failed to set the listening internet socket to non-blocking mode");
    }

    int flag = 1;
    if (setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)) == -1) {
        throw std::runtime_error("Failed to disable the Nagle algorithm");
    }
}

void InetSocket::destroy() {
    if (socketFd != INVALID_SOCKET_FD) {
        close(socketFd);
        socketFd = INVALID_SOCKET_FD;
    }
}

void InetSocket::connectToServer(const std::string &serverAddress) {
    socketFd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverSockAddr {};

    serverSockAddr.sin_family = AF_INET;
    serverSockAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverAddress.c_str(), &serverSockAddr.sin_addr);

    if (connect(socketFd, (struct sockaddr *)&serverSockAddr, sizeof(serverSockAddr)) == -1) {
        std::cerr << "Failed to connect to the Internet server." << std::endl;
        destroy();
        return;
    }
}
} // namespace echoserverclient