#include <condition_variable>
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <sys/socket.h>
#include <sys/stat.h>
#include <system_error>
#include <thread>

#include "../include/dispatcher.hpp"

namespace echoserver {

namespace {
volatile std::sig_atomic_t shutdownRequested = false;
} // namespace

Dispatcher::Dispatcher(int initServerCount) {
    servers.reserve(initServerCount);

    this->addListenerSocket(std::make_unique<echoserverclient::InetSocket>(startPort));
    this->addListenerSocket(std::make_unique<echoserverclient::UnixSocket>(startUnixPath));

    while (--initServerCount >= 0) {
        startServer();
    }

    inputToCommand["--set-response-schema"] = std::make_unique<ResponseSchemaCliCommand>(*this);
    inputToCommand["--help"] = std::make_unique<DispatcherHelpCliCommand>(*this);
}

Dispatcher::~Dispatcher() {
    for (int i = 0; i < servers.size(); i++) {
        kill(getServerPidAtIdx(i), SIGTERM);
    }
}

void Dispatcher::signalHandler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        shutdownRequested = true;
    }
}

void Dispatcher::start() {
    std::signal(SIGINT, Dispatcher::signalHandler);
    std::signal(SIGTERM, Dispatcher::signalHandler);

    prepareListenerSockets();

    std::jthread userInput([this](std::stop_token st) { this->cliInputHandler(st); });

    while (!shutdownRequested) {
        if (pollListenerSockets() != -1) {
            forwardIncomingConnections();
        }
    }

    userInput.request_stop();
    std::cout << "\nDispatcher terminated." << std::endl;
}

void Dispatcher::startServer() {
    latestServerId++;

    std::string pipeName = serverIdToPipePath(latestServerId);
    mkfifo(pipeName.c_str(), O_RDWR);

    int pid = fork();

    if (pid == -1) {
        latestServerId--;
        std::cerr << "Failed to start the new server process. " << errno << std::endl;
    } else if (pid == 0) {
        // Child process
        Server server(pipeName);
        server.addListenerSocket(std::make_unique<echoserverclient::UnixSocket>(serverIdToUnixPath(latestServerId)));
        server.addListenerSocket(std::make_unique<echoserverclient::InetSocket>(serverIdToPort(latestServerId)));
        server.start();
        exit(0);
    } else {
        // Parent process
        servers.push_back(std::make_pair(pid, latestServerId));
    }
}

void Dispatcher::forwardIncomingConnections() {
    for (size_t i = 0, end = listenerFds.size(); i < end; i++) {
        if (!(listenerFds[i].revents & POLLIN)) {
            continue;
        }

        // TODO: check if the process is still running?
        int nextIdx = (lastReceivingServerIdx++) % servers.size();
        int serverPid = getServerPidAtIdx(nextIdx);
        int pipeFd = open(serverIdToPipePath(getServerIdAtIdx(nextIdx)).c_str(), O_WRONLY);
        if (pipeFd == -1) {
            throw std::system_error(errno, std::generic_category(), "Failed to open the dispatcher->server pipe");
        }

        std::string newClientFdStr = std::to_string(listenerPool[i]->setupNewConnection());
        ssize_t bytesWritten = write(pipeFd, newClientFdStr.c_str(), newClientFdStr.size());
        if (bytesWritten == -1) {
            close(pipeFd);
            throw std::system_error(errno, std::generic_category(), "Failed to write to the dispatcher->server pipe");
        }
        
        close(pipeFd);
    }
}

void Dispatcher::cliInputHandler(std::stop_token token) {
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

void Dispatcher::setResponseSchema(AbstractResponseSchema schema) {
    // TODO
}
} // namespace echoserver