#include <condition_variable>
#include <csignal>
#include <iostream>
#include <memory>
#include <sys/socket.h>
#include <system_error>
#include <thread>

#include "../include/dispatcher.hpp"

namespace echoserver {

namespace {
volatile std::sig_atomic_t shutdownRequested = false;
} // namespace

Dispatcher::Dispatcher(int initServerCount) {
    serverPids.reserve(initServerCount);

    // Setup dispatcher listeners
    addListenerSocket(std::make_unique<echoserverclient::InetSocket>(startPort));
    addListenerSocket(std::make_unique<echoserverclient::UnixSocket>(startUnixPath));

    while (--initServerCount >= 0) {
        startServer();
    }

    // Setup available dispatcher CLI commands
    inputToCommand["--set-response-schema"] = std::make_unique<ResponseSchemaCliCommand>(*this);
    inputToCommand["--help"] = std::make_unique<DispatcherHelpCliCommand>(*this);
}

Dispatcher::~Dispatcher() {
    for (int pid : serverPids) {
        kill(pid, SIGTERM);
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

void Dispatcher::forwardIncomingConnections() {
    // TODO
}

void Dispatcher::startServer() {
    int pid = fork();
    if (pid == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to start the new server process");
    } else if (pid == 0) {
        // Child process
        Server server;
        serverCount++;
        server.addListenerSocket(std::make_unique<echoserverclient::UnixSocket>(startUnixPath + std::to_string(serverCount)));
        server.addListenerSocket(std::make_unique<echoserverclient::InetSocket>(startPort + serverCount));
        //TOOD: handle not being able to use current port number
        server.start();
        exit(0);
    } else {
        // Parent process
        serverCount++;
        serverPids.push_back(pid);
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

bool Dispatcher::executeCommand(echoserverclient::AbstractTokens tokens) {
    auto command = tokens->getOption();
    bool exists = inputToCommand.find(command) != inputToCommand.end();

    if (exists) {
        inputToCommand[command]->execute(std::move(tokens));
    }

    return exists;
}

void Dispatcher::setResponseSchema(AbstractResponseSchema schema) {
    // TODO
}
} // namespace echoserver