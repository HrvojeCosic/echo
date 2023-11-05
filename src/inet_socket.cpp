#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <netinet/tcp.h>
#include <stdexcept>

#include "../include/socket.hpp"

namespace echoserverclient {

InetSocket::InetSocket(int port) : port(port) {
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd == INVALID_SOCKET_FD) {
        throw std::system_error(errno, std::generic_category(), "Failed to create the internet socket");
    }

    int enable = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to set internet socket options");
    }
}

void InetSocket::bind() {
    struct sockaddr_in serverAddr {};

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (::bind(socketFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to bind the internet domain socket");
    }
}

int InetSocket::setupNewConnection() {
    struct sockaddr_in clientAddr {};

    socklen_t clientAddrLen = sizeof(clientAddr);

    int clientSocket = accept(socketFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET_FD) {
        throw std::system_error(errno, std::generic_category(), "Failed to accept a new internet socket connection");
    }
    initOptions(clientSocket);

    char newClientIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), newClientIp, INET_ADDRSTRLEN);
    std::cout << "Accepted connection from " << newClientIp << ":" << ntohs(clientAddr.sin_port) << std::endl;
    return clientSocket;
}

void InetSocket::initOptions(int socket) {
    if (fcntl(socketFd, F_SETFL, O_NONBLOCK) == -1) {
        throw std::system_error(errno, std::generic_category(),
                                "Failed to set the internet socket to non-blocking mode");
    }

    int flag = 1;
    if (setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)) == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to disable the Nagle algorithm");
    }
}

void InetSocket::connectToServer(const std::string &serverAddress) {
    struct sockaddr_in serverSockAddr {};

    serverSockAddr.sin_family = AF_INET;
    serverSockAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverAddress.c_str(), &serverSockAddr.sin_addr);

    if (connect(socketFd, (struct sockaddr *)&serverSockAddr, sizeof(serverSockAddr)) == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to connect to the Internet server");
    }
}
} // namespace echoserverclient