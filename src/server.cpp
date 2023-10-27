#include "../include/server.hpp"

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

Server::Server(int port, const std::string unixSocketPath) : port(port), unixSocketPath(unixSocketPath) {
    inetListenSocket = createAndBindInetSocket();
    unixListenSocket = createAndBindUnixSocket(unixSocketPath);

    if (fcntl(inetListenSocket, F_SETFL, O_NONBLOCK) == -1) {
        throw std::runtime_error("Failed to set the listening socket to non-blocking mode");
    }

    int flag = 1;
    if (setsockopt(inetListenSocket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)) == -1) {
        throw std::runtime_error("Failed to disable the Nagle algorithm");
    }
}

Server::~Server() {
    close(unixListenSocket);
    unlink(unixSocketPath.c_str());

    close(inetListenSocket);
    for (int clientSocket : clientSockets) {
        if (clientSocket != -1) {
            close(clientSocket);
        }
    }
}

int Server::createAndBindInetSocket() {
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
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

int Server::createAndBindUnixSocket(const std::string &socketPath) {
    int listenSocket = socket(AF_UNIX, SOCK_STREAM, 0);
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

void Server::start() {
    listen(inetListenSocket, maxClients);

    pollFds.emplace_back(pollfd{inetListenSocket, POLLIN, 0});
}