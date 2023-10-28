#include "../include/server.hpp"

#include <arpa/inet.h>
#include <cassert>
#include <cmath>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdexcept>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace echoserver {

Server::Server() {
    clientPool.reserve(maxClients);
    listenerPool.reserve(maxListeners);
    pollFds.reserve(maxClients + maxListeners);
}

Server::~Server() {
    for (AbstractSocket &listenerSock : listenerPool) {
        listenerSock->destroy();
    }

    for (int clientSock : clientPool) {
        if (clientSock != -1) {
            close(clientSock);
        }
    }
}

void Server::addListener(AbstractSocket listener) {
    int listenSocket = listener->createAndBind();
    listener->initOptions(listenSocket);

    pollFds.emplace(getNextListenerPollFdsIterator(), pollfd{listenSocket, POLLIN, 0});
    listenerPool.emplace_back(std::move(listener));
}

void Server::start() {
    for (AbstractSocket &listener : listenerPool) {
        listen(listener->getListenSocket(), maxClients);
    }

    while (true) {
        int numReady = poll(pollFds.data(), pollFds.size(), -1);

        if (numReady == -1) {
            if (errno == EINTR) { // Poll was interrupted
                continue;
            } else {
                throw std::runtime_error("Poll error");
            }
        }

        for (size_t i = 0, end = listenerPool.size(); i < end; i++) {
            if (pollFds[i].revents & POLLIN) {
                int newClientSock = listenerPool[i]->setupNewConnection();
                clientPool.emplace_back(newClientSock);
                pollFds.emplace_back(pollfd{newClientSock, POLLIN, 0});
            }
        }

        for (size_t i = listenerPool.size(), end = pollFds.size(); i < end; i++) {
            if (pollFds[i].revents & (POLLIN | POLLHUP)) {
                handleClientData(i);
            }
        }
    }
}

void Server::handleClientData(int pollFdIdx) {
    int clientSocket = pollFds[pollFdIdx].fd;
    char buffer[bufferSize];
    int bytesRead = recv(clientSocket, buffer, bufferSize, 0);

    if (bytesRead == -1) {
        if (errno != EWOULDBLOCK) {
            std::cerr << "Error reading from client " << pollFdIdx << std::endl;
            closeClientConnection(pollFdIdx);
        }
    } else if (bytesRead == 0) {
        closeClientConnection(pollFdIdx);
        std::cout << "Connection closed by a client " << std::endl;
    } else {
        std::string data(buffer, bytesRead);
        std::string response = "Echo: " + data;
        send(clientSocket, response.c_str(), response.length(), 0);
    }
}

void Server::closeClientConnection(int pollFdIdx) {
    int clientSocket = pollFds[pollFdIdx].fd;
    close(clientSocket);

    pollFds.erase(pollFds.begin() + pollFdIdx);
    clientPool.erase(pollFdIdxToClientPoolIterator(pollFdIdx));
}
} // namespace echoserver