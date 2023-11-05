#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/un.h>

#include "../include/socket.hpp"

namespace echoserverclient {

UnixSocket::UnixSocket(const std::string path) : socketPath(path) {
    socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketFd == INVALID_SOCKET_FD) {
        throw std::system_error(errno, std::generic_category(), "Failed to create the Unix domain socket");
    }
}

void UnixSocket::bind() {
    struct sockaddr_un serverAddr {};

    serverAddr.sun_family = AF_UNIX;
    strncpy(serverAddr.sun_path, socketPath.c_str(), sizeof(serverAddr.sun_path));

    if (::bind(socketFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to bind the Unix domain socket");
    }
}

int UnixSocket::setupNewConnection() {
    struct sockaddr_in clientAddr {};

    socklen_t clientAddrLen = sizeof(clientAddr);

    int clientSocket = accept(socketFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET_FD) {
        throw std::system_error(errno, std::generic_category(), "Failed to accept a new unix socket connection");
    }
    initOptions(clientSocket);

    std::cout << "Accepted connection from " << socketPath << std::endl;
    return clientSocket;
}

void UnixSocket::initOptions(int socket) {
    if (fcntl(socket, F_SETFL, O_NONBLOCK) == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to set the unix socket to non-blocking mode");
    }
}

void UnixSocket::connectToServer(const std::string &serverAddress) {
    struct sockaddr_un serverSockAddr {};

    serverSockAddr.sun_family = AF_UNIX;
    strcpy(serverSockAddr.sun_path, serverAddress.c_str());

    if (connect(socketFd, (struct sockaddr *)&serverSockAddr, sizeof(serverSockAddr)) == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to connect to the Unix Domain server");
    }
}
} // namespace echoserverclient