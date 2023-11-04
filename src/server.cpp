#include <csignal>
#include <iostream>
#include <sys/socket.h>
#include <system_error>
#include <thread>

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

    std::signal(SIGINT, Server::signalHandler);
    std::signal(SIGTERM, Server::signalHandler);
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
    try {
        listener->bind();
        listener->initOptions(listener->getsocketFd());
        pollFds.emplace(getNextListenerPollFdsIterator(), pollfd{listener->getsocketFd(), POLLIN, 0});
        listenerPool.emplace_back(std::move(listener));
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }
}

bool Server::executeCommand(echoserverclient::AbstractTokens tokens) {
    auto command = tokens->getOption();
    bool exists = inputToCommand.find(command) != inputToCommand.end();

    if (exists) {
        inputToCommand[command]->execute(std::move(tokens));
    }

    return exists;
}

void Server::start() {
    std::jthread userInput([this](std::stop_token st) { this->cliInputHandler(st); });

    try {
        prepareListeners();
        while (!serverShutdownRequested) {
            pollFileDescriptors();
        }
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }

    userInput.request_stop();
    std::cout << std::endl << "Shutting down the server... press enter to continue" << std::endl;
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

void Server::pollFileDescriptors() {
    if (poll(pollFds.data(), pollFds.size(), -1) == -1) {
        if (errno != EINTR) {
            throw std::system_error(errno, std::generic_category(), "Poll error");
        }
    } else {
        acceptIncomingConnections();
        handleIncomingData();
    }
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
            // all is good, send back the response
            std::string data(buffer, bytesRead);
            responseSchema->generateResponse(data);
            std::string response = data + "\n";
            send(clientSocket, response.c_str(), response.length(), 0);
        }
    }
}

void Server::closeClientConnection(int pollFdIdx) {
    int clientSocket = pollFds[pollFdIdx].fd;
    close(clientSocket);

    pollFds.erase(pollFds.begin() + pollFdIdx);
    clientPool.erase(pollFdIdxToClientPoolIterator(pollFdIdx));
}
} // namespace echoserver