#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "../include/socket.hpp"

namespace echoserver {

int UnixSocket::createAndBind() {
    listenSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listenSocket == -1) {
        throw std::runtime_error("Failed to create the Unix domain listening socket");
    }

    struct sockaddr_un serverAddr {};

    serverAddr.sun_family = AF_UNIX;
    strncpy(serverAddr.sun_path, socketPath.c_str(), sizeof(serverAddr.sun_path));

    if (bind(listenSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        throw std::runtime_error("Failed to bind the Unix domain listening socket");
    }

    return listenSocket;
}

int UnixSocket::setupNewConnection() {
    while (true) {
        struct sockaddr_in clientAddr {};

        socklen_t clientAddrLen = sizeof(clientAddr);

        int clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
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

void UnixSocket::destroy() {
    close(listenSocket);
    unlink(socketPath.c_str());
}
} // namespace echoserver