#include <csignal>
#include <iostream>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

#include "../include/server.hpp"

namespace echoserver {

namespace {
volatile std::sig_atomic_t serverShutdownRequested = false;
const int maxClients = 50; // denotes max number of clients per listener
} // namespace

void Server::signalHandler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        serverShutdownRequested = true;
    }
}

Server::Server() : responseSchema(std::make_unique<EquivalentResponseSchema>()) {
    clientPool.reserve(maxClients);
    pollFds.reserve(maxClients);
}

Server::~Server() {
    for (auto pollFd : pollFds) {
        close(pollFd.fd);
    }
}

void Server::addListener(echoserverclient::AbstractSocket listener) {
    try {
        listener->bind();
        listener->initOptions(listener->getsocketFd());
        pollFds.emplace(getNextListenerPollFdsIterator(), pollfd{listener->getsocketFd(), POLLIN, 0});
        listenerPool.emplace_back(std::move(listener));
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }
}

void Server::start() {
    std::signal(SIGINT, Server::signalHandler);
    std::signal(SIGTERM, Server::signalHandler);
    try {
        prepareListeners();
        while (!serverShutdownRequested) {
            if (pollFileDescriptors() != -1) {
                acceptIncomingConnections();
                handleIncomingData();
            }
        }
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }
}

int Server::pollFileDescriptors() {
    int numReady = poll(pollFds.data(), pollFds.size(), -1);
    if (numReady == -1 && errno != EINTR) {
        throw std::system_error(errno, std::generic_category(), "Poll error");
    }
    return numReady;
}

void Server::prepareListeners() {
    for (echoserverclient::AbstractSocket &listener : listenerPool) {
        if (listen(listener->getsocketFd(), maxClients) == -1) {
            throw std::system_error(errno, std::generic_category(),
                                    "Failed to prepare for conection acceptance from listener socket");
        }
    }
}

void Server::acceptIncomingConnections() {
    for (size_t i = 0, end = listenerPool.size(); i < end; i++) {
        if (pollFds[i].revents & POLLIN) {
            int newClientSock = listenerPool[i]->setupNewConnection();
            clientPool.emplace_back(newClientSock);
            pollFds.emplace_back(pollfd{newClientSock, POLLIN, 0});
        }
    }
}

void Server::handleIncomingData() {
    for (size_t pollFdIdx = listenerPool.size(), end = pollFds.size(); pollFdIdx < end; pollFdIdx++) {
        if (!(pollFds[pollFdIdx].revents & (POLLIN | POLLHUP))) {
            continue;
        }

        int clientSocket = pollFds[pollFdIdx].fd;
        char buffer[echoserverclient::bufferSize];
        int bytesRead = receiveFromClient(pollFdIdx, buffer);

        if (bytesRead == 0) {
            closeClientConnection(pollFdIdx);
            std::cout << "Connection closed by a client " << std::endl;
        } else {
            std::string data(buffer, bytesRead);
            responseSchema->generateResponse(data);
            std::string response = data + "\n";
            send(clientSocket, response.c_str(), response.length(), 0);
        }
    }
}

int Server::receiveFromClient(int pollFdIdx, char *buffer) {
    int bytesRead = recv(pollFds[pollFdIdx].fd, buffer, echoserverclient::bufferSize, 0);
    if (bytesRead == -1) {
        throw std::system_error(errno, std::generic_category(), "Error reading from client");
    }
    return bytesRead;
}

void Server::closeClientConnection(int pollFdIdx) {
    int clientSocket = pollFds[pollFdIdx].fd;

    if (close(clientSocket) == -1) {
        throw std::system_error(errno, std::generic_category(), "Error closing client connection");
    }

    pollFds.erase(pollFds.begin() + pollFdIdx);
    clientPool.erase(pollFdIdxToClientPoolIterator(pollFdIdx));
}
} // namespace echoserver