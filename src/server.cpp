#include "../include/server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

Server::Server(int port, const std::string unixSocketPath) : port(port), unixSocketPath(unixSocketPath) {
    constexpr int vecSize = maxClients + maxListeners;
    socketPool.reserve(vecSize);
    pollFds.reserve(vecSize);

    inetListenSocket = createAndBindInetSocket();
    unixListenSocket = createAndBindUnixSocket(unixSocketPath);
    initInetSocketOptions(inetListenSocket);
}

Server::~Server() {
    close(unixListenSocket);
    unlink(unixSocketPath.c_str());

    close(inetListenSocket);
    for (int socket : socketPool) {
        if (socket != -1) {
            close(socket);
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

void Server::acceptNewInetConnection() {
    while (true) {
        struct sockaddr_in clientAddr {};

        socklen_t clientAddrLen = sizeof(clientAddr);

        int clientSocket = accept(inetListenSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                break; // No more incoming connections
            } else {
                throw std::runtime_error("Failed to accept a new connection");
            }
        }

        initInetSocketOptions(clientSocket);

        pollFds.emplace_back(pollfd{clientSocket, POLLIN, 0});
        socketPool.emplace_back(clientSocket);

        char newClientIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), newClientIp, INET_ADDRSTRLEN);
        std::cout << "Accepted connection from " << newClientIp << ":" << ntohs(clientAddr.sin_port) << std::endl;
    }
}

void Server::acceptNewUnixConnection() {
    while (true) {
        struct sockaddr_in clientAddr {};

        socklen_t clientAddrLen = sizeof(clientAddr);

        int clientSocket = accept(unixListenSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                break; // No more incoming connections
            } else {
                throw std::runtime_error("Failed to accept a new connection");
            }
        }

        initInetSocketOptions(
            clientSocket); // nagle's doesn't make sense for unix (TODO: separate inet and unix alltogether)

        pollFds.emplace_back(pollfd{clientSocket, POLLIN, 0});
        socketPool.emplace_back(clientSocket);

        std::cout << "Accepted connection from " << unixSocketPath << std::endl;
    }
}

void Server::initInetSocketOptions(int socket) {
    if (fcntl(inetListenSocket, F_SETFL, O_NONBLOCK) == -1) {
        throw std::runtime_error("Failed to set the listening socket to non-blocking mode");
    }

    int flag = 1;
    if (setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)) == -1) {
        throw std::runtime_error("Failed to disable the Nagle algorithm");
    }
}

void Server::start() {
    listen(inetListenSocket, maxClients);

    pollFds.emplace_back(pollfd{inetListenSocket, POLLIN, 0});

    while (true) {
        int numReady = poll(pollFds.data(), pollFds.size(), -1);

        if (numReady == -1) {
            if (errno == EINTR) { // Poll was interrupted
                continue;
            } else {
                throw std::runtime_error("Poll error");
            }
        }

        if (pollFds[0].revents & POLLIN) {
            acceptNewInetConnection();
        }

        for (size_t i = 1, max = pollFds.size(); i < max; i++) {
            if (pollFds[i].revents & (POLLIN | POLLHUP)) {
                handleClientData(i);
            }
        }
    }
}

void Server::handleClientData(int clientIdx) {
    int clientSocket = pollFds[clientIdx].fd;
    char buffer[bufferSize];
    int bytesRead = recv(clientSocket, buffer, bufferSize, 0);

    if (bytesRead == -1) {
        if (errno != EWOULDBLOCK) {
            std::cerr << "Error reading from client " << clientIdx << std::endl;
            closeClientConnection(clientIdx);
        }
    } else if (bytesRead == 0) {
        std::cout << "Connection closed by client " << clientIdx << std::endl;
        closeClientConnection(clientIdx);
    } else {
        std::string data(buffer, bytesRead);
        std::string response = "Echo: " + data;
        send(clientSocket, response.c_str(), response.length(), 0);
    }
}

void Server::closeClientConnection(int clientIdx) {
    int clientSocket = pollFds[clientIdx].fd;
    close(clientSocket);

    pollFds.erase(pollFds.begin() + clientIdx);
    socketPool.erase(socketPool.begin() + clientIdx);
}
