#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

#include "listener/server.hpp"

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
    int pipeFd = open(pipeName.c_str(), O_RDWR);

    if (pipeFd == -1) {
        throw std::system_error(errno, std::generic_category(), "Server pipe opening error");
    }

    fdsToPoll.emplace_back(pollfd{pipeFd, POLLIN, 0});

    int fcntlRes = fcntl(pipeFd, F_SETFL, fcntl(pipeFd, F_GETFL) | O_NONBLOCK);
    if (fcntlRes == -1) {
        throw std::system_error(errno, std::generic_category(), "Setting pipe mode to non-blocking error");
    }

    responseSchema = std::make_unique<EquivalentResponseSchema>();
}

Server::~Server() {
    for (int i = getClientFdStartIdx(); i < fdsToPoll.size(); i++) {
        close(fdsToPoll[i].fd);
    }

    close(getPipePollFd().fd);
    unlink(pipeName.c_str());
}

void Server::start() {
    std::signal(SIGINT, Server::signalHandler);
    std::signal(SIGTERM, Server::signalHandler);

    try {
        prepareListenerSockets();

        while (!serverShutdownRequested) {
            if (pollFileDescriptors() > 0) {
                handleIncomingData();
            }
        }
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }
}

void Server::acceptIncomingClientConnections() {
    char pipeBuf[sizeof(int)];

    while (true) {
        ssize_t bytesRead = read(getPipePollFd().fd, pipeBuf, sizeof(pipeBuf));
        bool hasDataAvailable = bytesRead != 0 && !(bytesRead == -1 && errno == EAGAIN);

        if (!hasDataAvailable) {
            break;
        } else if (bytesRead == -1) {
            throw std::system_error(errno, std::generic_category(), "Reading client socket fd from the pipe error");
        }

        int newClientFd = duplicateClientFd(std::stoi(pipeBuf));
        clientPool.emplace_back(newClientFd);
        fdsToPoll.emplace_back(pollfd{newClientFd, POLLIN, 0});
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

int Server::pollFileDescriptors() {
    int numReady = poll(fdsToPoll.data(), fdsToPoll.size(), -1);
    if (numReady == -1 && errno != EINTR) {
        throw std::system_error(errno, std::generic_category(), "Server poll fds error");
    }
    return numReady;
}

void Server::handleIncomingData() {
    if (getPipePollFd().revents & POLLIN) {
        acceptIncomingClientConnections();
    }

    for (size_t i = getClientFdStartIdx(), end = fdsToPoll.size(); i < end; i++) {
        if (!(fdsToPoll[i].revents & (POLLIN | POLLHUP))) {
            continue;
        }

        int clientSocket = fdsToPoll[i].fd;
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

int Server::receiveFromClient(int pollFdIdx, char *buffer) {
    int bytesRead = recv(fdsToPoll[pollFdIdx].fd, buffer, echoserverclient::bufferSize, 0);
    if (bytesRead == -1) {
        throw std::system_error(errno, std::generic_category(), "Error reading from client");
    }
    return bytesRead;
}

void Server::closeClientConnection(int pollFdIdx) {
    int clientSocket = fdsToPoll[pollFdIdx].fd;

    if (close(clientSocket) == -1) {
        throw std::system_error(errno, std::generic_category(), "Error closing client connection");
    }

    fdsToPoll.erase(fdsToPoll.begin() + pollFdIdx);
    clientPool.erase(clientPool.begin() + (pollFdIdx - getClientFdStartIdx()));
}
} // namespace echoserver