#include <array>
#include <condition_variable>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <system_error>
#include <thread>
#include <unistd.h>

#include "listener/dispatcher.hpp"

namespace echo {

namespace {
volatile std::sig_atomic_t shutdownRequested = false;
} // namespace

Dispatcher::Dispatcher(int initServerCount) {
    servers.reserve(initServerCount);

    this->addListenerSocket(std::make_unique<InetSocket>(startPort));
    this->addListenerSocket(std::make_unique<UnixSocket>(startUnixPath));

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
    chmod(pipeName.c_str(), S_IRUSR | S_IWUSR);

    int pid = fork();

    if (pid == -1) {
        latestServerId--;
        std::cerr << "Failed to start the new server process. " << errno << std::endl;
    } else if (pid == 0) {
        // Child process
        // After the server is finished running, clean it up and exit out of the child process
        {
            Server server(pipeName);
            server.addListenerSocket(std::make_unique<UnixSocket>(serverIdToUnixPath(latestServerId)));
            server.addListenerSocket(std::make_unique<InetSocket>(serverIdToPort(latestServerId)));
            server.start();
        }
        std::exit(0);
    } else {
        // Parent process

        // Disable Yama security module for the parent process
        // https://manpages.ubuntu.com/manpages/focal/en/man2/prctl.2.html
        // https://stackoverflow.com/questions/75045206/eperm-on-pidfd-getfd-with-socket/76114536#76114536
        prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);

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

        int clientFd = listenerPool[i]->setupNewConnection();

        std::array<std::byte, sizeof(std::byte) + sizeof(int)> clientFdBuf;
        clientFdBuf[0] = newClientFdPipeFlag;
        std::memcpy(&clientFdBuf[sizeof(std::byte)], &clientFd, sizeof(int));

        ssize_t bytesWritten = write(pipeFd, &clientFdBuf, sizeof(clientFdBuf));
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

        auto tokens = std::make_unique<RuntimeTokens>(userInput);
        auto optionToken = tokens->getOption();
        if (inputToCommand.contains(optionToken)) {
            inputToCommand[optionToken]->execute(std::move(tokens));
        } else {
            inputToCommand["--help"]->execute(std::move(tokens));
        }
    }
}

bool Dispatcher::executeCommand(AbstractTokens tokens) {
    auto command = tokens->getOption();
    bool exists = inputToCommand.find(command) != inputToCommand.end();

    if (exists) {
        inputToCommand[command]->execute(std::move(tokens));
    }

    return exists;
}

} // namespace echo