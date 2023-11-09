#include <csignal>
#include <iostream>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

#include "../include/server.hpp"

namespace echoserver {

namespace {
volatile std::sig_atomic_t serverShutdownRequested = false;
const int maxClients = 50;
} // namespace

void Server::signalHandler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        serverShutdownRequested = true;
    }
}

Server::~Server() {
    for (auto clientFd : clientFds) {
        close(clientFd.fd);
    }
}

void Server::start() {
    std::signal(SIGINT, Server::signalHandler);
    std::signal(SIGTERM, Server::signalHandler);
    try {
        prepareListenerSockets();
        while (!serverShutdownRequested) {
            if (pollListenerSockets() != -1) {
                acceptIncomingConnections();
            }
            if (pollClientSockets() != -1) {
                handleIncomingData();
            }
        }
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }
}

int Server::pollClientSockets() {
    int numReady = poll(clientFds.data(), clientFds.size(), -1);
    if (numReady == -1 && errno != EINTR) {
        throw std::system_error(errno, std::generic_category(), "Client socket poll error");
    }
    return numReady;
}

void Server::acceptIncomingConnections() {
    for (size_t i = 0, end = listenerPool.size(); i < end; i++) {
        if (listenerFds[i].revents & POLLIN) {
            int newClientSock = listenerPool[i]->setupNewConnection();
            clientPool.emplace_back(newClientSock);
            listenerFds.emplace_back(pollfd{newClientSock, POLLIN, 0});
        }
    }
}

void Server::handleIncomingData() {
    for (size_t i = 0, end = clientFds.size(); i < end; i++) {
        if (!(clientFds[i].revents & (POLLIN | POLLHUP))) {
            continue;
        }

        int clientSocket = clientFds[i].fd;
        char buffer[echoserverclient::bufferSize];
        int bytesRead = receiveFromClient(i, buffer);

        if (bytesRead == 0) {
            closeClientConnection(i);
            std::cout << "Connection closed by a client " << std::endl;
        } else {
            std::string data(buffer, bytesRead);
            responseSchema->generateResponse(data);
            std::string response = data + "\n";
            send(clientSocket, response.c_str(), response.length(), 0);
        }
    }
}

int Server::receiveFromClient(int idx, char *buffer) {
    int bytesRead = recv(clientFds[idx].fd, buffer, echoserverclient::bufferSize, 0);
    if (bytesRead == -1) {
        throw std::system_error(errno, std::generic_category(), "Error reading from client");
    }
    return bytesRead;
}

void Server::closeClientConnection(int idx) {
    int clientSocket = clientFds[idx].fd;

    if (close(clientSocket) == -1) {
        throw std::system_error(errno, std::generic_category(), "Error closing client connection");
    }

    clientFds.erase(clientFds.begin() + idx);
    clientPool.erase(clientPool.begin() + idx);
}
} // namespace echoserver