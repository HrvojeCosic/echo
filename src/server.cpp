#include <csignal>
#include <fcntl.h>
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

Server::Server(std::string pipeName) : pipeName(pipeName) {
    pipeFd = open(pipeName.c_str(), O_RDWR);

    if (pipeFd == -1) {
        throw std::system_error(errno, std::generic_category(), "Server pipe opening error");
    }

    int fcntlRes = fcntl(pipeFd, F_SETFL, fcntl(pipeFd, F_GETFL) | O_NONBLOCK);
    if (fcntlRes == -1) {
        throw std::system_error(errno, std::generic_category(), "Setting pipe mode to non-blocking error");
    }

    responseSchema = std::make_unique<EquivalentResponseSchema>();
}

Server::~Server() {
    for (auto clientFd : clientFds) {
        close(clientFd.fd);
    }
    close(pipeFd);
    unlink(pipeName.c_str());
}

void Server::start() {
    std::signal(SIGINT, Server::signalHandler);
    std::signal(SIGTERM, Server::signalHandler);

    try {
        prepareListenerSockets();
        while (!serverShutdownRequested) {
            // TODO: see if it makes sense to combine two poll processes (pipe and client sockets) with a timeout into
            // one blocking poll process
            acceptIncomingClientConnections();

            if (pollClientSockets() > 0) {
                handleIncomingData();
            }
        }
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }
}

// Warning: this returns without blocking/timeout if there's no new data on the pipe
void Server::acceptIncomingClientConnections() {
    char pipeBuf[sizeof(int)];

    while (true) {
        ssize_t bytesRead = read(pipeFd, pipeBuf, sizeof(pipeBuf));
        bool hasDataAvailable = bytesRead != 0 && !(bytesRead == -1 && errno == EAGAIN);

        if (!hasDataAvailable) {
            break;
        } else if (bytesRead == -1) {
            throw std::system_error(errno, std::generic_category(), "Reading client socket fd from the pipe error");
        }

        int newClientFd = duplicateClientFd(std::stoi(pipeBuf));
        clientPool.emplace_back(newClientFd);
        clientFds.emplace_back(pollfd{newClientFd, POLLIN, 0});
    }
}

int Server::duplicateClientFd(int fd) {
    int ppidFd = syscall(SYS_pidfd_open, getppid(), 0);
    if (ppidFd == -1) {
        throw std::system_error(errno, std::generic_category(), "Getting parent pidfd error");
    }

    int clientFd = syscall(SYS_pidfd_getfd, ppidFd, fd, 0);
    if (clientFd == -1) {
        throw std::system_error(errno, std::generic_category(), "Duplicating client fd error");
    }

    return clientFd;
}

int Server::pollClientSockets() {
    int numReady = poll(clientFds.data(), clientFds.size(), 1000);
    if (numReady == -1 && errno != EINTR) {
        throw std::system_error(errno, std::generic_category(), "Client socket poll error");
    }
    return numReady;
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