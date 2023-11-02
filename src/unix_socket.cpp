#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/un.h>

#include "../include/socket.hpp"

namespace echoserverclient {

int UnixSocket::createAndBind() {
    socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketFd == INVALID_SOCKET_FD) {
        throw std::runtime_error("Failed to create the Unix domain listening socket");
    }

    struct sockaddr_un serverAddr {};

    serverAddr.sun_family = AF_UNIX;
    strncpy(serverAddr.sun_path, socketPath.c_str(), sizeof(serverAddr.sun_path));

    if (bind(socketFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        throw std::runtime_error("Failed to bind the Unix domain listening socket");
    }

    return socketFd;
}

int UnixSocket::setupNewConnection() {
    while (true) {
        struct sockaddr_in clientAddr {};

        socklen_t clientAddrLen = sizeof(clientAddr);

        int clientSocket = accept(socketFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET_FD) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                return clientSocket; // No more incoming connections
            } else {
                throw std::runtime_error("Failed to accept a new unix socket connection");
            }
        }

        initOptions(clientSocket);
        std::cout << "Accepted connection from " << socketPath << std::endl;

        return clientSocket;
    }
}

void UnixSocket::initOptions(int socket) {
    if (fcntl(socket, F_SETFL, O_NONBLOCK) == -1) {
        throw std::runtime_error("Failed to set the listening unix socket to non-blocking mode");
    }
}

void UnixSocket::connectToServer(const std::string &serverAddress) {
    socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketFd == INVALID_SOCKET_FD) {
        throw std::runtime_error("Failed to create the Unix domain client socket");
    }

    struct sockaddr_un serverSockAddr {};

    serverSockAddr.sun_family = AF_UNIX;
    strcpy(serverSockAddr.sun_path, serverAddress.c_str());

    if (connect(socketFd, (struct sockaddr *)&serverSockAddr, sizeof(serverSockAddr)) == -1) {
        std::cerr << "Failed to connect to the Unix Domain server. " << std::strerror(errno) << std::endl;
        return;
    }
}
} // namespace echoserverclient