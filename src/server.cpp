#include <arpa/inet.h>
#include <cassert>
#include <cmath>
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <ostream>
#include <stdexcept>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

#include "../include/response_schema_factory.hpp"
#include "../include/server.hpp"

namespace echoserver {

volatile std::sig_atomic_t serverShutdownRequested = false;

void Server::signalHandler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        serverShutdownRequested = true;
    }
}

Server::Server() : responseSchema(std::make_unique<EquivalentResponseSchema>()) {
    clientPool.reserve(maxClients);
    listenerPool.reserve(maxListeners);
    pollFds.reserve(maxClients + maxListeners);

    inputToCommand["--set-response-schema"] = std::make_unique<ResponseSchemaCliCommand>(*this);
    inputToCommand["--help"] = std::make_unique<ServerHelpCliCommand>(*this);
}

Server::~Server() {
    for (echoserverclient::AbstractSocket &listenerSock : listenerPool) {
        auto *unixListener = dynamic_cast<echoserverclient::UnixSocket *>(listenerSock.get());
        if (unixListener) {
            unlink(unixListener->getSocketPath().c_str());
        }
    }

    for (int clientSock : clientPool) {
        if (clientSock != -1) {
            close(clientSock);
        }
    }
}

Server &Server::getInstance() {
    static Server instance;
    return instance;
}

void Server::addListener(echoserverclient::AbstractSocket listener) {
    int listenSocket = listener->createAndBind();
    listener->initOptions(listenSocket);

    pollFds.emplace(getNextListenerPollFdsIterator(), pollfd{listenSocket, POLLIN, 0});
    listenerPool.emplace_back(std::move(listener));
}

bool Server::executeCommand(std::string command, echoserverclient::AbstractTokens tokens) {
    bool exists = inputToCommand.find(command) != inputToCommand.end();

    if (exists) {
        inputToCommand[command]->execute(std::move(tokens));
    }

    return exists;
}

void Server::cliInputHandler(std::stop_token token) {
    while (!token.stop_requested()) {
        std::string userInput;
        std::getline(std::cin, userInput);

        if (userInput.length() == 0)
            continue;

        auto tokens = std::make_unique<echoserverclient::RuntimeTokens>(userInput, ' ');
        auto optionToken = tokens->getOption();
        if (inputToCommand.contains(optionToken)) {
            inputToCommand[optionToken]->execute(std::move(tokens));
        } else {
            inputToCommand["--help"]->execute(std::move(tokens));
        }
    }
}

void Server::start() {
    std::jthread userInput([this](std::stop_token st) { this->cliInputHandler(st); });

    std::signal(SIGINT, Server::signalHandler);
    std::signal(SIGTERM, Server::signalHandler);

    for (echoserverclient::AbstractSocket &listener : listenerPool) {
        listen(listener->getsocketFd(), maxClients);
    }

    std::cout << "Server started." << std::endl;
    while (!serverShutdownRequested) {
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

    userInput.request_stop();
    std::cout << std::endl << "Shutting down the server... press enter to continue" << std::endl;
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
        responseSchema->generateResponse(data);
        std::string response = data + "\n";
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